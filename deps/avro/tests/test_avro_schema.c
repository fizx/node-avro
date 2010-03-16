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

#include "avro_private.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>

int test_cases = 0;
avro_writer_t avro_stderr;

static void run_tests(char *dirpath, int should_pass)
{
	char jsontext[4096];
	size_t jsonlen, rval;
	char filepath[1024];
	DIR *dir;
	struct dirent *dent;
	FILE *fp;
	avro_schema_t schema;
	avro_schema_error_t avro_schema_error;

	dir = opendir(dirpath);
	if (dir == NULL) {
		fprintf(stderr, "Unable to open '%s'\n", dirpath);
		exit(EXIT_FAILURE);
	}
	do {
		dent = readdir(dir);
		if (dent && dent->d_name[0] != '.') {
			int test_rval;
			snprintf(filepath, sizeof(filepath), "%s/%s", dirpath,
				 dent->d_name);
			fprintf(stderr, "TEST %s...", filepath);
			jsonlen = 0;
			fp = fopen(filepath, "r");
			if (!fp) {
				fprintf(stderr, "can't open!\n");
				exit(EXIT_FAILURE);
			}
			rval = fread(jsontext, 1, sizeof(jsontext) - 1, fp);
			jsontext[rval] = '\0';
			test_rval =
			    avro_schema_from_json(jsontext, jsonlen, &schema,
						  &avro_schema_error);
			test_cases++;
			if (test_rval == 0) {
				if (should_pass) {
					avro_schema_t schema_copy =
					    avro_schema_copy(schema);
					fprintf(stderr, "pass\n");
					avro_schema_to_json(schema,
							    avro_stderr);
					fprintf(stderr, "\n");
					if (!avro_schema_equal
					    (schema, schema_copy)) {
						fprintf(stderr,
							"failed to avro_schema_equal(schema,avro_schema_copy())\n");
						exit(EXIT_FAILURE);
					}
					avro_schema_decref(schema_copy);
					avro_schema_decref(schema);
				} else {
					/*
					 * Unexpected success 
					 */
					fprintf(stderr,
						"fail! (shouldn't succeed but did)\n");
					exit(EXIT_FAILURE);
				}
			} else {
				if (should_pass) {
					fprintf(stderr,
						"fail! (should have succeeded but didn't)\n");
					exit(EXIT_FAILURE);
				} else {
					fprintf(stderr, "pass\n");
				}
			}
		}
	}
	while (dent != NULL);
}

int main(int argc, char *argv[])
{
	char *srcdir = getenv("srcdir");
	char path[1024];

	AVRO_UNUSED(argc);
	AVRO_UNUSED(argv);

	if (!srcdir) {
		srcdir = ".";
	}

	avro_stderr = avro_writer_file(stderr);

	/*
	 * Run the tests that should pass 
	 */
	snprintf(path, sizeof(path), "%s/schema_tests/pass", srcdir);
	fprintf(stderr, "RUNNING %s\n", path);
	run_tests(path, 1);
	snprintf(path, sizeof(path), "%s/schema_tests/fail", srcdir);
	fprintf(stderr, "RUNNING %s\n", path);
	run_tests(path, 0);

	fprintf(stderr, "==================================================\n");
	fprintf(stderr,
		"Finished running %d schema test cases successfully \n",
		test_cases);
	fprintf(stderr, "==================================================\n");

	avro_writer_free(avro_stderr);
	return EXIT_SUCCESS;
}
