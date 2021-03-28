// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * src/report.c
 *
 * Match output support in different formats
 *
 * Copyright (C) 2018-2021 SCANOSS.COM
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "debug.h"
#include "report.h"
#include "quality.h"
#include "cryptography.h"
#include "vulnerability.h"
#include "cyclonedx.h"
#include "spdx.h"
#include "util.h"
#include "dependency.h"
#include "license.h"
#include "copyright.h"
#include "limits.h"

int report_format = plain;

/* Open JSON report */
void json_open()
{
	if (quiet) return;

	switch(report_format)
	{
		case plain:
			printf("{\n");
			break;

		case cyclonedx:
			cyclonedx_open();
			break;

		case spdx:
			spdx_open();
			break;
	}
}

/* Close main report */
void report_close()
{
	if (quiet) return;

	switch(report_format)
	{
		case plain:
			printf("}\n");
			break;

		case cyclonedx:
			cyclonedx_close();
			break;

		case spdx:
			spdx_close();
			break;

		case spdx_xml:
			spdx_xml_close();
			break;
	}
}

void json_open_file(char *filename)
{    
	if (quiet) return;

	switch(report_format)
	{
		case plain:
			printf("  \"%s\": [\n", filename);
			break;
	}
}

void json_close_file()
{
	if (quiet) return;

	switch(report_format)
	{
		case plain:
			printf("  ]\n");
			break;
	}
}

void print_json_nomatch(scan_data scan)
{
	if (quiet) return;

	double elapsed = (microseconds_now() - scan.timer);
	printf("    {\n");
	printf("      \"id\": \"none\",\n");
	printf("      \"elapsed\": \"%.6fs\"\n", elapsed / 1000000);
	printf("    }\n");
	fflush(stdout);
}

void print_json_match_plain(scan_data scan, match_data match)
{
	printf("    {\n");
	printf("      \"id\": \"%s\",\n", matchtypes[match.type]);
	printf("      \"lines\": \"%s\",\n", scan.line_ranges);
	printf("      \"oss_lines\": \"%s\",\n", scan.oss_ranges);

	if ((engine_flags & ENABLE_SNIPPET_IDS) && match.type == snippet)
	{
		printf("      \"snippet_ids\": \"%s\",\n", scan.snippet_ids);
	}

	printf("      \"matched\": \"%s\",\n", scan.matched_percent);
	printf("      \"vendor\": \"%s\",\n", match.vendor);
	printf("      \"component\": \"%s\",\n", match.component);
	printf("      \"version\": \"%s\",\n", match.version);
	printf("      \"latest\": \"%s\",\n", match.latest_version);

	printf("      \"url\": \"%s\",\n", match.url);
	printf("      \"release_date\": \"%s\",\n", match.release_date);
	printf("      \"file\": \"%s\",\n", match.file);

	char *url_id = md5_hex(match.url_md5);
	printf("      \"url_hash\": \"%s\",\n", url_id);
	free(url_id);

	char *file_id = md5_hex(match.file_md5);
	printf("      \"file_hash\": \"%s\",\n", file_id);
	free(file_id);

	if (!(engine_flags & DISABLE_DEPENDENCIES))
	{
		printf("      \"dependencies\": ");
		print_dependencies(match);
	}

	if (!(engine_flags & DISABLE_LICENSES))
	{
		printf("      \"licenses\": ");
		print_licenses(match);
	}

	if (!(engine_flags & DISABLE_COPYRIGHTS))
	{
		printf("      \"copyrights\": ");
		print_copyrights(match);
	}

	if (!(engine_flags & DISABLE_VULNERABILITIES))
	{
		printf("      \"vulnerabilities\": ");
		print_vulnerabilities(match);
	}

	if (!(engine_flags & DISABLE_QUALITY))
	{
		printf("      \"quality\": ");
		print_quality(match);
	}

	if (!(engine_flags & DISABLE_CRIPTOGRAPHY))
	{
		printf("      \"cryptography\": ");
		print_cryptography(match);
	}

	double elapsed = microseconds_now() - scan.timer;
	printf("      \"elapsed\": \"%.6fs\"\n", elapsed / 1000000);

	printf("    }\n");
	fflush(stdout);
}

void print_match(scan_data scan, match_data match)
{
	/* Calculate component/vendor md5 for aggregated data queries */
	vendor_component_md5(match.vendor, match.component, match.pair_md5);

	if (quiet) return;

	switch(report_format)
	{

		case plain:
			print_json_match_plain(scan, match);
			break;

		case cyclonedx:
			print_json_match_cyclonedx(scan, match);
			break;

		case spdx:
			print_json_match_spdx(scan, match);
			break;

		case spdx_xml:
			print_xml_match_spdx(scan, match);
			break;
	}
}

void report_open(scan_data *scan)
{
	if (report_format != spdx_xml) json_open();
	else spdx_xml_open(scan);
}

