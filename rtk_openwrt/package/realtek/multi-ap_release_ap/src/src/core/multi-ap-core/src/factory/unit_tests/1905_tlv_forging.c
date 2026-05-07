/*
 *  Broadband Forum IEEE 1905.1/1a stack
 *
 *  Copyright (c) 2017, Broadband Forum
 *  Copyright (c) 2018, Realtek Semiconductor Corp.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  Subject to the terms and conditions of this license, each copyright
 *  holder and contributor hereby grants to those receiving rights under
 *  this license a perpetual, worldwide, non-exclusive, no-charge,
 *  royalty-free, irrevocable (except for failure to satisfy the
 *  conditions of this license) patent license to make, have made, use,
 *  offer to sell, sell, import, and otherwise transfer this software,
 *  where such license applies only to those patent claims, already
 *  acquired or hereafter acquired, licensable by such copyright holder or
 *  contributor that are necessarily infringed by:
 *
 *  (a) their Contribution(s) (the licensed copyrights of copyright holders
 *      and non-copyrightable additions of contributors, in source or binary
 *      form) alone; or
 *
 *  (b) combination of their Contribution(s) with the work of authorship to
 *      which such Contribution(s) was added by such copyright holder or
 *      contributor, if, at the time the Contribution is added, such addition
 *      causes such combination to be necessarily infringed. The patent
 *      license shall not apply to any other combinations which include the
 *      Contribution.
 *
 *  Except as expressly stated above, no rights or licenses from any
 *  copyright holder or contributor is granted under this license, whether
 *  expressly, by implication, estoppel or otherwise.
 *
 *  DISCLAIMER
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 *  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 *  TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 *  PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 *  DAMAGE.
 */

//
// This file tests the "forge_1905_TLV_from_structure()" function by providing
// some test input structures and checking the generated output stream.
//

#include "platform.h"
#include "1905_tlvs.h"
#include "1905_tlv_test_vectors.h"

INT8U _check(const char *test_description, INT8U *input, INT8U *expected_output, INT16U expected_output_len)
{
	INT8U  result;
	INT8U *real_output;
	INT16U real_output_len;

	real_output = forge_1905_TLV_from_structure((INT8U *)input, &real_output_len, 0);

	if (NULL == real_output) {
		PLATFORM_PRINTF("%-100s: KO !!!\n", test_description);
		PLATFORM_PRINTF("  forge_1905_TLV_from_structure() returned a NULL pointer\n");

		return 1;
	}

	if ((expected_output_len == real_output_len) && (0 == PLATFORM_MEMCMP(expected_output, real_output, real_output_len))) {
		result = 0;
		PLATFORM_PRINTF("%-100s: OK\n", test_description);
	} else {
		INT16U i;

		result = 1;
		PLATFORM_PRINTF("%-100s: KO !!!\n", test_description);
		PLATFORM_PRINTF("  Expected output: ");
		for (i = 0; i < expected_output_len; i++) {
			PLATFORM_PRINTF("%02x ", expected_output[i]);
		}
		PLATFORM_PRINTF("\n");
		PLATFORM_PRINTF("  Real output    : ");
		for (i = 0; i < real_output_len; i++) {
			PLATFORM_PRINTF("%02x ", real_output[i]);
		}
		PLATFORM_PRINTF("\n");
	}

	return result;
}

int main(void)
{
	INT8U result = 0;

#define x1905TLVFORGE001 "x1905TLVFORGE001 - Forge link metric query TLV (x1905_tlv_structure_001)"
	result += _check(x1905TLVFORGE001, (INT8U *)&x1905_tlv_structure_001, x1905_tlv_stream_001, x1905_tlv_stream_len_001);

#define x1905TLVFORGE002 "x1905TLVFORGE002 - Forge link metric query TLV (x1905_tlv_structure_002)"
	result += _check(x1905TLVFORGE002, (INT8U *)&x1905_tlv_structure_002, x1905_tlv_stream_002, x1905_tlv_stream_len_002);

#define x1905TLVFORGE003 "x1905TLVFORGE003 - Forge transmitter link metric TLV (x1905_tlv_structure_004)"
	result += _check(x1905TLVFORGE003, (INT8U *)&x1905_tlv_structure_004, x1905_tlv_stream_004, x1905_tlv_stream_len_004);

#define x1905TLVFORGE004 "x1905TLVFORGE004 - Forge transmitter link metric TLV (x1905_tlv_structure_005)"
	result += _check(x1905TLVFORGE004, (INT8U *)&x1905_tlv_structure_005, x1905_tlv_stream_005, x1905_tlv_stream_len_005);

#define x1905TLVFORGE005 "x1905TLVFORGE005 - Forge receiver link metric TLV (x1905_tlv_structure_006)"
	result += _check(x1905TLVFORGE005, (INT8U *)&x1905_tlv_structure_006, x1905_tlv_stream_006, x1905_tlv_stream_len_006);

#define x1905TLVFORGE006 "x1905TLVFORGE006 - Forge receiver link metric TLV (x1905_tlv_structure_007)"
	result += _check(x1905TLVFORGE006, (INT8U *)&x1905_tlv_structure_007, x1905_tlv_stream_007, x1905_tlv_stream_len_007);

#define x1905TLVFORGE007 "x1905TLVFORGE007 - Forge AL MAC address type TLV (x1905_tlv_structure_008)"
	result += _check(x1905TLVFORGE007, (INT8U *)&x1905_tlv_structure_008, x1905_tlv_stream_008, x1905_tlv_stream_len_008);

#define x1905TLVFORGE008 "x1905TLVFORGE008 - Forge MAC address type TLV (x1905_tlv_structure_009)"
	result += _check(x1905TLVFORGE008, (INT8U *)&x1905_tlv_structure_009, x1905_tlv_stream_009, x1905_tlv_stream_len_009);

#define x1905TLVFORGE009 "x1905TLVFORGE009 - Forge device information type TLV (x1905_tlv_structure_010)"
	result += _check(x1905TLVFORGE009, (INT8U *)&x1905_tlv_structure_010, x1905_tlv_stream_010, x1905_tlv_stream_len_010);

#define x1905TLVFORGE010 "x1905TLVFORGE010 - Forge device bridging capability TLV (x1905_tlv_structure_011)"
	result += _check(x1905TLVFORGE010, (INT8U *)&x1905_tlv_structure_011, x1905_tlv_stream_011, x1905_tlv_stream_len_011);

#define x1905TLVFORGE011 "x1905TLVFORGE011 - Forge device bridging capability TLV (x1905_tlv_structure_012)"
	result += _check(x1905TLVFORGE011, (INT8U *)&x1905_tlv_structure_012, x1905_tlv_stream_012, x1905_tlv_stream_len_012);

#define x1905TLVFORGE012 "x1905TLVFORGE012 - Forge device bridging capability TLV (x1905_tlv_structure_013)"
	result += _check(x1905TLVFORGE012, (INT8U *)&x1905_tlv_structure_013, x1905_tlv_stream_013, x1905_tlv_stream_len_013);

#define x1905TLVFORGE013 "x1905TLVFORGE013 - Forge non 1905 neighbor device list TLV (x1905_tlv_structure_014)"
	result += _check(x1905TLVFORGE013, (INT8U *)&x1905_tlv_structure_014, x1905_tlv_stream_014, x1905_tlv_stream_len_014);

#define x1905TLVFORGE014 "x1905TLVFORGE014 - Forge non 1905 neighbor device list TLV (x1905_tlv_structure_015)"
	result += _check(x1905TLVFORGE014, (INT8U *)&x1905_tlv_structure_015, x1905_tlv_stream_015, x1905_tlv_stream_len_015);

#define x1905TLVFORGE015 "x1905TLVFORGE015 - Forge neighbor device list TLV (x1905_tlv_structure_016)"
	result += _check(x1905TLVFORGE015, (INT8U *)&x1905_tlv_structure_016, x1905_tlv_stream_016, x1905_tlv_stream_len_016);

#define x1905TLVFORGE016 "x1905TLVFORGE016 - Forge neighbor device list TLV (x1905_tlv_structure_017)"
	result += _check(x1905TLVFORGE016, (INT8U *)&x1905_tlv_structure_017, x1905_tlv_stream_017, x1905_tlv_stream_len_017);

#define x1905TLVFORGE017 "x1905TLVFORGE017 - Forge link metric result code TLV (x1905_tlv_structure_018)"
	result += _check(x1905TLVFORGE017, (INT8U *)&x1905_tlv_structure_018, x1905_tlv_stream_018, x1905_tlv_stream_len_018);

#define x1905TLVFORGE018 "x1905TLVFORGE018 - Forge link metric result code TLV (x1905_tlv_structure_020)"
	result += _check(x1905TLVFORGE018, (INT8U *)&x1905_tlv_structure_020, x1905_tlv_stream_020, x1905_tlv_stream_len_020);

#define x1905TLVFORGE019 "x1905TLVFORGE019 - Forge autoconfig freq band TLV (x1905_tlv_structure_022)"
	result += _check(x1905TLVFORGE019, (INT8U *)&x1905_tlv_structure_022, x1905_tlv_stream_022, x1905_tlv_stream_len_022);

#define x1905TLVFORGE020 "x1905TLVFORGE020 - Forge supported role TLV (x1905_tlv_structure_024)"
	result += _check(x1905TLVFORGE020, (INT8U *)&x1905_tlv_structure_024, x1905_tlv_stream_024, x1905_tlv_stream_len_024);

#define x1905TLVFORGE021 "x1905TLVFORGE021 - Forge supported freq band TLV (x1905_tlv_structure_026)"
	result += _check(x1905TLVFORGE021, (INT8U *)&x1905_tlv_structure_026, x1905_tlv_stream_026, x1905_tlv_stream_len_026);

#define x1905TLVFORGE022 "x1905TLVFORGE022 - Forge push button event notification TLV (x1905_tlv_structure_028)"
	result += _check(x1905TLVFORGE022, (INT8U *)&x1905_tlv_structure_028, x1905_tlv_stream_028, x1905_tlv_stream_len_028);

#define x1905TLVFORGE035 "x1905TLVFORGE035 - Forge vendor specific TLV (x1905_tlv_structure_041)"
	result += _check(x1905TLVFORGE035, (INT8U *)&x1905_tlv_structure_041, x1905_tlv_stream_041, x1905_tlv_stream_len_041);

	// Return the number of test cases that failed
	//
	return result;
}
