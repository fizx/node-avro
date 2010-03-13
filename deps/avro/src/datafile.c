/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to you under the Apache License, Version 2.0 
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.  See the License for the specific language governing
 * permissions and limitations under the License. 
 */
#include <avro.h>
#include "avro_private.h"
#include "encoding.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

struct avro_file_reader_t {
	avro_schema_t writers_schema;
	avro_reader_t reader;
	char sync[16];
	int64_t blocks_read;
	int64_t blocks_total;
	int64_t current_blocklen;
};

struct avro_file_writer_t {
	avro_schema_t writers_schema;
	avro_writer_t writer;
	char sync[16];
	int block_count;
	avro_writer_t datum_writer;
	char datum_buffer[16 * 1024];
};

/* TODO: should we just read /dev/random? */
static void generate_sync(struct avro_file_writer_t *w)
{
	int i;
	srand(time(NULL));
	for (i = 0; i < sizeof(w->sync); i++) {
		w->sync[i] = ((double)rand() / (RAND_MAX + 1.0)) * 255;
	}
}

static int write_sync(struct avro_file_writer_t *w)
{
	return avro_write(w->writer, w->sync, sizeof(w->sync));
}

static int write_header(struct avro_file_writer_t *w)
{
	int rval;
	uint8_t version = 1;
	/* TODO: remove this static buffer */
	avro_writer_t schema_writer;
	char schema_buf[64 * 1024];
	const avro_encoding_t *enc = &avro_binary_encoding;

	/* Generate random sync */
	generate_sync(w);

	check(rval, avro_write(w->writer, "Obj", 3));
	check(rval, avro_write(w->writer, &version, 1));

	check(rval, enc->write_long(w->writer, 3));
	check(rval, enc->write_string(w->writer, "avro.sync"));
	check(rval, enc->write_bytes(w->writer, w->sync, sizeof(w->sync)));
	check(rval, enc->write_string(w->writer, "avro.codec"));
	check(rval, enc->write_bytes(w->writer, "null", 4));
	check(rval, enc->write_string(w->writer, "avro.schema"));
	schema_writer = avro_writer_memory(schema_buf, sizeof(schema_buf));
	rval = avro_schema_to_json(w->writers_schema, schema_writer);
	if (rval) {
		avro_writer_free(schema_writer);
		return rval;
	}
	check(rval,
	      enc->write_bytes(w->writer, schema_buf,
			       avro_writer_tell(schema_writer)));
	check(rval, enc->write_long(w->writer, 0));
	return write_sync(w);
}

static int
file_writer_init_fp(const char *path, const char *mode,
		    struct avro_file_writer_t *w)
{
	FILE *fp = fopen(path, mode);
	if (!fp) {
		return ENOMEM;
	}
	w->writer = avro_writer_file(fp);
	if (!w->writer) {
		return ENOMEM;
	}
	return 0;
}

static int
file_writer_create(const char *path, avro_schema_t schema,
		   struct avro_file_writer_t *w)
{
	int rval = file_writer_init_fp(path, "wx", w);
	if (rval) {
		check(rval, file_writer_init_fp(path, "w", w));
	}

	w->datum_writer =
	    avro_writer_memory(w->datum_buffer, sizeof(w->datum_buffer));
	if (!w->datum_writer) {
		avro_writer_free(w->writer);
		return ENOMEM;
	}

	w->writers_schema = schema;
	return write_header(w);
}

int
avro_file_writer_create(const char *path, avro_schema_t schema,
			avro_file_writer_t * writer)
{
	struct avro_file_writer_t *w;
	int rval;
	if (!path || !is_avro_schema(schema) || !writer) {
		return EINVAL;
	}
	w = malloc(sizeof(struct avro_file_writer_t));
	if (!w) {
		return ENOMEM;
	}
	rval = file_writer_create(path, schema, w);
	if (rval) {
		free(w);
		return rval;
	}
	*writer = w;
	return 0;
}

static int file_read_header(avro_reader_t reader,
			    avro_schema_t * writers_schema, char *sync,
			    int synclen)
{
	int rval;
	avro_schema_t meta_schema;
	avro_schema_t meta_values_schema;
	avro_datum_t meta;
	char magic[4];
	avro_datum_t schema_bytes;
	char *p;
	int64_t len;
	avro_schema_error_t schema_error;

	check(rval, avro_read(reader, magic, sizeof(magic)));
	if (magic[0] != 'O' || magic[1] != 'b' || magic[2] != 'j'
	    || magic[3] != 1) {
		return EILSEQ;
	}
	meta_values_schema = avro_schema_bytes();
	meta_schema = avro_schema_map(meta_values_schema);
	rval = avro_read_data(reader, meta_schema, NULL, &meta);
	if (rval) {
		return EILSEQ;
	}
	check(rval, avro_map_get(meta, "avro.schema", &schema_bytes));
	avro_bytes_get(schema_bytes, &p, &len);
	check(rval,
	      avro_schema_from_json(p, len, writers_schema, &schema_error));
	avro_schema_decref(meta);
	return avro_read(reader, sync, synclen);
}

static int file_writer_open(const char *path, struct avro_file_writer_t *w)
{
	int rval;
	FILE *fp;
	avro_reader_t reader;

	fp = fopen(path, "r");
	if (!fp) {
		return errno;
	}
	reader = avro_reader_file(fp);
	if (!reader) {
		return ENOMEM;
	}
	rval =
	    file_read_header(reader, &w->writers_schema, w->sync,
			     sizeof(w->sync));
	avro_reader_free(reader);
	/* Position to end of file and get ready to write */
	rval = file_writer_init_fp(path, "a", w);
	if (rval) {
		free(w);
	}
	return rval;
}

int avro_file_writer_open(const char *path, avro_file_writer_t * writer)
{
	struct avro_file_writer_t *w;
	int rval;
	if (!path || !writer) {
		return EINVAL;
	}
	w = malloc(sizeof(struct avro_file_writer_t));
	if (!w) {
		return ENOMEM;
	}
	rval = file_writer_open(path, w);
	if (rval) {
		free(w);
		return rval;
	}

	*writer = w;
	return 0;
}

static int file_read_block_count(avro_file_reader_t r)
{
	int rval;
	const avro_encoding_t *enc = &avro_binary_encoding;
	check(rval, enc->read_long(r->reader, &r->blocks_total));
	check(rval, enc->read_long(r->reader, &r->current_blocklen));
	r->blocks_read = 0;
	return 0;
}

int avro_file_reader(const char *path, avro_file_reader_t * reader)
{
	int rval;
	FILE *fp;
	struct avro_file_reader_t *r =
	    malloc(sizeof(struct avro_file_reader_t));
	if (!r) {
		return ENOMEM;
	}

	fp = fopen(path, "r");
	if (!fp) {
		return errno;
	}
	r->reader = avro_reader_file(fp);
	if (!r->reader) {
		return ENOMEM;
	}
	rval = file_read_header(r->reader, &r->writers_schema, r->sync,
				sizeof(r->sync));
	if (rval == 0) {
		rval = file_read_block_count(r);
		if (rval == 0) {
			*reader = r;
		}
	}
	return rval;
}

static int file_write_block(avro_file_writer_t w)
{
	const avro_encoding_t *enc = &avro_binary_encoding;
	int rval;

	if (w->block_count) {
		int64_t blocklen = avro_writer_tell(w->datum_writer);
		/* Write the block count */
		check(rval, enc->write_long(w->writer, w->block_count));
		/* Write the block length */
		check(rval, enc->write_long(w->writer, blocklen));
		/* Write the block */
		check(rval, avro_write(w->writer, w->datum_buffer, blocklen));
		/* Write the sync marker */
		check(rval, write_sync(w));
		/* Reset the datum writer */
		avro_writer_reset(w->datum_writer);
		w->block_count = 0;
	}
	return 0;
}

int avro_file_writer_append(avro_file_writer_t w, avro_datum_t datum)
{
	int rval;
	if (!w || !datum) {
		return EINVAL;
	}
	rval = avro_write_data(w->datum_writer, w->writers_schema, datum);
	if (rval) {
		check(rval, file_write_block(w));
		rval =
		    avro_write_data(w->datum_writer, w->writers_schema, datum);
		if (rval) {
			/* TODO: if the datum encoder larger than our buffer,
			   just write a single large datum */
			return rval;
		}
	}
	w->block_count++;
	return 0;
}

int avro_file_writer_sync(avro_file_writer_t w)
{
	return file_write_block(w);
}

int avro_file_writer_flush(avro_file_writer_t w)
{
	int rval;
	check(rval, file_write_block(w));
	avro_writer_flush(w->writer);
	return 0;
}

int avro_file_writer_close(avro_file_writer_t w)
{
	int rval;
	check(rval, avro_file_writer_flush(w));
	avro_writer_free(w->writer);
	return 0;
}

int avro_file_reader_read(avro_file_reader_t r, avro_schema_t readers_schema,
			  avro_datum_t * datum)
{
	int rval;
	char sync[16];

	if (!r || !datum) {
		return EINVAL;
	}

	check(rval,
	      avro_read_data(r->reader, r->writers_schema, readers_schema,
			     datum));
	r->blocks_read++;

	if (r->blocks_read == r->blocks_total) {
		check(rval, avro_read(r->reader, sync, sizeof(sync)));
		if (memcmp(r->sync, sync, sizeof(r->sync)) != 0) {
			/* wrong sync bytes */
			return EILSEQ;
		}
		/* For now, ignore errors (e.g. EOF) */
		file_read_block_count(r);
	}
	return 0;
}

int avro_file_reader_close(avro_file_reader_t reader)
{
	avro_reader_free(reader->reader);
	return 0;
}
