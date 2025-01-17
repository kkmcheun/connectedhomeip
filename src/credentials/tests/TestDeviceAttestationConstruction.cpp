/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
#include <credentials/DeviceAttestationConstructor.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/Span.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

using namespace chip;
using namespace chip::Credentials;

static void TestAttestationElements_Roundtrip(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::Platform::ScopedMemoryBuffer<uint8_t> attestationElements;
    size_t attestationElementsLen;

    uint8_t certificationDeclaration[] = { 0xd2, 0x84, 0x4b, 0xa2, 0x01, 0x26, 0x04, 0x46, 0x63, 0x73, 0x61, 0x63, 0x64, 0x30,
                                           0xa0, 0x58, 0x1d, 0x15, 0x25, 0x01, 0x88, 0x99, 0x25, 0x02, 0xfe, 0xff, 0x25, 0x03,
                                           0xd2, 0x04, 0x25, 0x04, 0x2e, 0x16, 0x24, 0x05, 0xaa, 0x25, 0x06, 0xde, 0xc0, 0x25,
                                           0x07, 0x94, 0x26, 0x18, 0x58, 0x40, 0x96, 0x57, 0x2d, 0xd6, 0x3c, 0x03, 0x64, 0x0b,
                                           0x28, 0x67, 0x02, 0xbd, 0x6b, 0xba, 0x48, 0xac, 0x7c, 0x83, 0x54, 0x9b, 0x68, 0x73,
                                           0x29, 0x47, 0x48, 0xb9, 0x51, 0xd5, 0xab, 0x66, 0x62, 0x2e, 0x9d, 0x26, 0x10, 0x41,
                                           0xf8, 0x0e, 0x97, 0x49, 0xfe, 0xff, 0x78, 0x10, 0x02, 0x49, 0x67, 0xae, 0xdf, 0x41,
                                           0x38, 0x36, 0x5b, 0x0a, 0x22, 0x57, 0x14, 0x9c, 0x9a, 0x12, 0x3e, 0x0d, 0x30, 0xaa };
    uint8_t attestationNonce[] = { 0xe0, 0x42, 0x1b, 0x91, 0xc6, 0xfd, 0xcd, 0xb4, 0x0e, 0x2a, 0x4d, 0x2c, 0xf3, 0x1d, 0xb2, 0xb4,
                                   0xe1, 0x8b, 0x41, 0x1b, 0x1d, 0x3a, 0xd4, 0xd1, 0x2a, 0x9d, 0x90, 0xaa, 0x8e, 0x52, 0xfa, 0xe2 };
    uint32_t timestamp         = 677103357;
    uint8_t vendorReserved1[]  = { 0x73, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x5f, 0x76, 0x65, 0x6e, 0x64, 0x6f,
                                  0x72, 0x5f, 0x72, 0x65, 0x73, 0x65, 0x72, 0x76, 0x65, 0x64, 0x31 };
    uint8_t vendorReserved3[]  = { 0x76, 0x65, 0x6e, 0x64, 0x6f, 0x72, 0x5f, 0x72, 0x65, 0x73, 0x65, 0x72,
                                  0x76, 0x65, 0x64, 0x33, 0x5f, 0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65 };
    ByteSpan vendorReservedArray[] = { ByteSpan(vendorReserved1), ByteSpan(vendorReserved3) };
    uint16_t vendorId              = 0xbeef;
    uint16_t profileNum            = 0xdead;

    attestationElementsLen = sizeof(certificationDeclaration) + sizeof(attestationNonce) + sizeof(timestamp) +
        sizeof(vendorReserved1) + sizeof(vendorReserved3) + sizeof(uint64_t) * 5;
    attestationElements.Alloc(attestationElementsLen);
    NL_TEST_ASSERT(inSuite, attestationElements);

    {
        MutableByteSpan attestationElementsSpan(attestationElements.Get(), attestationElementsLen);

        // Test wrong size nonce
        err = ConstructAttestationElements(
            ByteSpan(certificationDeclaration), ByteSpan(attestationNonce, sizeof(attestationNonce) - 1), timestamp, ByteSpan(),
            vendorReservedArray, ArraySize(vendorReservedArray), vendorId, profileNum, attestationElementsSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);

        // Test with missing mandatory TLV entries
        err = ConstructAttestationElements(ByteSpan(), ByteSpan(attestationNonce), timestamp, ByteSpan(), vendorReservedArray,
                                           ArraySize(vendorReservedArray), vendorId, profileNum, attestationElementsSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);

        // Test with missing mandatory TLV entries
        err =
            ConstructAttestationElements(ByteSpan(certificationDeclaration), ByteSpan(), timestamp, ByteSpan(), vendorReservedArray,
                                         ArraySize(vendorReservedArray), vendorId, profileNum, attestationElementsSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);

        // Test for success with entirely valid arguments
        err = ConstructAttestationElements(ByteSpan(certificationDeclaration), ByteSpan(attestationNonce), timestamp, ByteSpan(),
                                           vendorReservedArray, ArraySize(vendorReservedArray), vendorId, profileNum,
                                           attestationElementsSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        attestationElementsLen = attestationElementsSpan.size();
    }

    ByteSpan certificationDeclarationSpan;
    ByteSpan attestationNonceSpan;
    uint32_t timestampDeconstructed;
    ByteSpan firmwareInfoSpan;
    ByteSpan vendorReservedDeconstructed[2];
    size_t vendorReservedDeconstructedSize = ArraySize(vendorReservedDeconstructed);
    uint16_t vendorIdDeconstructed;
    uint16_t profileNumDeconstructed;

    err =
        DeconstructAttestationElements(ByteSpan(attestationElements.Get(), attestationElementsLen), certificationDeclarationSpan,
                                       attestationNonceSpan, timestampDeconstructed, firmwareInfoSpan, vendorReservedDeconstructed,
                                       vendorReservedDeconstructedSize, vendorIdDeconstructed, profileNumDeconstructed);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, certificationDeclarationSpan.data_equal(ByteSpan(certificationDeclaration)));
    NL_TEST_ASSERT(inSuite, attestationNonceSpan.data_equal(ByteSpan(attestationNonce)));
    NL_TEST_ASSERT(inSuite, timestamp == timestampDeconstructed);
    NL_TEST_ASSERT(inSuite, firmwareInfoSpan.empty());
    NL_TEST_ASSERT(inSuite, ArraySize(vendorReservedArray) == ArraySize(vendorReservedDeconstructed));
    for (size_t i = 0; i < ArraySize(vendorReservedArray); ++i)
    {
        NL_TEST_ASSERT(inSuite, vendorReservedArray[i].data_equal(vendorReservedDeconstructed[i]));
    }
    NL_TEST_ASSERT(inSuite, vendorIdDeconstructed == vendorId);
    NL_TEST_ASSERT(inSuite, profileNumDeconstructed == profileNum);
}

static void TestAttestationElements_Construction(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    Platform::ScopedMemoryBuffer<uint8_t> attestationElements;
    size_t attestationElementsLen;

    uint8_t attestationElementsTestVector[] = {
        0x15, 0x30, 0x01, 0x70, 0xd2, 0x84, 0x4b, 0xa2, 0x01, 0x26, 0x04, 0x46, 0x63, 0x73, 0x61, 0x63, 0x64, 0x30, 0xa0,
        0x58, 0x1d, 0x15, 0x25, 0x01, 0x88, 0x99, 0x25, 0x02, 0xfe, 0xff, 0x25, 0x03, 0xd2, 0x04, 0x25, 0x04, 0x2e, 0x16,
        0x24, 0x05, 0xaa, 0x25, 0x06, 0xde, 0xc0, 0x25, 0x07, 0x94, 0x26, 0x18, 0x58, 0x40, 0x96, 0x57, 0x2d, 0xd6, 0x3c,
        0x03, 0x64, 0x0b, 0x28, 0x67, 0x02, 0xbd, 0x6b, 0xba, 0x48, 0xac, 0x7c, 0x83, 0x54, 0x9b, 0x68, 0x73, 0x29, 0x47,
        0x48, 0xb9, 0x51, 0xd5, 0xab, 0x66, 0x62, 0x2e, 0x9d, 0x26, 0x10, 0x41, 0xf8, 0x0e, 0x97, 0x49, 0xfe, 0xff, 0x78,
        0x10, 0x02, 0x49, 0x67, 0xae, 0xdf, 0x41, 0x38, 0x36, 0x5b, 0x0a, 0x22, 0x57, 0x14, 0x9c, 0x9a, 0x12, 0x3e, 0x0d,
        0x30, 0xaa, 0x30, 0x02, 0x20, 0xe0, 0x42, 0x1b, 0x91, 0xc6, 0xfd, 0xcd, 0xb4, 0x0e, 0x2a, 0x4d, 0x2c, 0xf3, 0x1d,
        0xb2, 0xb4, 0xe1, 0x8b, 0x41, 0x1b, 0x1d, 0x3a, 0xd4, 0xd1, 0x2a, 0x9d, 0x90, 0xaa, 0x8e, 0x52, 0xfa, 0xe2, 0x26,
        0x03, 0xfd, 0xc6, 0x5b, 0x28, 0xd0, 0xf1, 0xff, 0x3e, 0x00, 0x01, 0x00, 0x17, 0x73, 0x61, 0x6d, 0x70, 0x6c, 0x65,
        0x5f, 0x76, 0x65, 0x6e, 0x64, 0x6f, 0x72, 0x5f, 0x72, 0x65, 0x73, 0x65, 0x72, 0x76, 0x65, 0x64, 0x31, 0xd0, 0xf1,
        0xff, 0x3e, 0x00, 0x03, 0x00, 0x18, 0x76, 0x65, 0x6e, 0x64, 0x6f, 0x72, 0x5f, 0x72, 0x65, 0x73, 0x65, 0x72, 0x76,
        0x65, 0x64, 0x33, 0x5f, 0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x18
    };
    uint8_t certificationDeclaration[] = { 0xd2, 0x84, 0x4b, 0xa2, 0x01, 0x26, 0x04, 0x46, 0x63, 0x73, 0x61, 0x63, 0x64, 0x30,
                                           0xa0, 0x58, 0x1d, 0x15, 0x25, 0x01, 0x88, 0x99, 0x25, 0x02, 0xfe, 0xff, 0x25, 0x03,
                                           0xd2, 0x04, 0x25, 0x04, 0x2e, 0x16, 0x24, 0x05, 0xaa, 0x25, 0x06, 0xde, 0xc0, 0x25,
                                           0x07, 0x94, 0x26, 0x18, 0x58, 0x40, 0x96, 0x57, 0x2d, 0xd6, 0x3c, 0x03, 0x64, 0x0b,
                                           0x28, 0x67, 0x02, 0xbd, 0x6b, 0xba, 0x48, 0xac, 0x7c, 0x83, 0x54, 0x9b, 0x68, 0x73,
                                           0x29, 0x47, 0x48, 0xb9, 0x51, 0xd5, 0xab, 0x66, 0x62, 0x2e, 0x9d, 0x26, 0x10, 0x41,
                                           0xf8, 0x0e, 0x97, 0x49, 0xfe, 0xff, 0x78, 0x10, 0x02, 0x49, 0x67, 0xae, 0xdf, 0x41,
                                           0x38, 0x36, 0x5b, 0x0a, 0x22, 0x57, 0x14, 0x9c, 0x9a, 0x12, 0x3e, 0x0d, 0x30, 0xaa };
    uint8_t attestationNonce[] = { 0xe0, 0x42, 0x1b, 0x91, 0xc6, 0xfd, 0xcd, 0xb4, 0x0e, 0x2a, 0x4d, 0x2c, 0xf3, 0x1d, 0xb2, 0xb4,
                                   0xe1, 0x8b, 0x41, 0x1b, 0x1d, 0x3a, 0xd4, 0xd1, 0x2a, 0x9d, 0x90, 0xaa, 0x8e, 0x52, 0xfa, 0xe2 };
    uint32_t timestamp         = 677103357;
    uint8_t vendorReserved1[]  = { 0x73, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x5f, 0x76, 0x65, 0x6e, 0x64, 0x6f,
                                  0x72, 0x5f, 0x72, 0x65, 0x73, 0x65, 0x72, 0x76, 0x65, 0x64, 0x31 };
    uint8_t vendorReserved3[]  = { 0x76, 0x65, 0x6e, 0x64, 0x6f, 0x72, 0x5f, 0x72, 0x65, 0x73, 0x65, 0x72,
                                  0x76, 0x65, 0x64, 0x33, 0x5f, 0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65 };
    ByteSpan vendorReservedArray[] = { ByteSpan(vendorReserved1), ByteSpan(), ByteSpan(vendorReserved3) };
    uint16_t vendorId              = 0xFFF1;
    uint16_t profileNum            = 0x003E;

    attestationElementsLen = sizeof(certificationDeclaration) + sizeof(attestationNonce) + sizeof(timestamp) +
        sizeof(vendorReserved1) + sizeof(vendorReserved3) + sizeof(uint64_t) * 5;
    attestationElements.Alloc(attestationElementsLen);
    NL_TEST_ASSERT(inSuite, attestationElements);

    {
        MutableByteSpan attestationElementsSpan(attestationElements.Get(), attestationElementsLen);

        err = ConstructAttestationElements(ByteSpan(certificationDeclaration), ByteSpan(attestationNonce), timestamp, ByteSpan(),
                                           vendorReservedArray, ArraySize(vendorReservedArray), vendorId, profileNum,
                                           attestationElementsSpan);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        attestationElementsLen = attestationElementsSpan.size();

        NL_TEST_ASSERT(inSuite, attestationElementsSpan.data_equal(ByteSpan(attestationElementsTestVector)));
    }
}

static void TestAttestationElements_Deconstruction(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    uint8_t attestationElementsTestVector[] = {
        0x15, 0x30, 0x01, 0x70, 0xd2, 0x84, 0x4b, 0xa2, 0x01, 0x26, 0x04, 0x46, 0x63, 0x73, 0x61, 0x63, 0x64, 0x30, 0xa0,
        0x58, 0x1d, 0x15, 0x25, 0x01, 0x88, 0x99, 0x25, 0x02, 0xfe, 0xff, 0x25, 0x03, 0xd2, 0x04, 0x25, 0x04, 0x2e, 0x16,
        0x24, 0x05, 0xaa, 0x25, 0x06, 0xde, 0xc0, 0x25, 0x07, 0x94, 0x26, 0x18, 0x58, 0x40, 0x96, 0x57, 0x2d, 0xd6, 0x3c,
        0x03, 0x64, 0x0b, 0x28, 0x67, 0x02, 0xbd, 0x6b, 0xba, 0x48, 0xac, 0x7c, 0x83, 0x54, 0x9b, 0x68, 0x73, 0x29, 0x47,
        0x48, 0xb9, 0x51, 0xd5, 0xab, 0x66, 0x62, 0x2e, 0x9d, 0x26, 0x10, 0x41, 0xf8, 0x0e, 0x97, 0x49, 0xfe, 0xff, 0x78,
        0x10, 0x02, 0x49, 0x67, 0xae, 0xdf, 0x41, 0x38, 0x36, 0x5b, 0x0a, 0x22, 0x57, 0x14, 0x9c, 0x9a, 0x12, 0x3e, 0x0d,
        0x30, 0xaa, 0x30, 0x02, 0x20, 0xe0, 0x42, 0x1b, 0x91, 0xc6, 0xfd, 0xcd, 0xb4, 0x0e, 0x2a, 0x4d, 0x2c, 0xf3, 0x1d,
        0xb2, 0xb4, 0xe1, 0x8b, 0x41, 0x1b, 0x1d, 0x3a, 0xd4, 0xd1, 0x2a, 0x9d, 0x90, 0xaa, 0x8e, 0x52, 0xfa, 0xe2, 0x26,
        0x03, 0xfd, 0xc6, 0x5b, 0x28, 0xd0, 0xf1, 0xff, 0x3e, 0x00, 0x01, 0x00, 0x17, 0x73, 0x61, 0x6d, 0x70, 0x6c, 0x65,
        0x5f, 0x76, 0x65, 0x6e, 0x64, 0x6f, 0x72, 0x5f, 0x72, 0x65, 0x73, 0x65, 0x72, 0x76, 0x65, 0x64, 0x31, 0xd0, 0xf1,
        0xff, 0x3e, 0x00, 0x03, 0x00, 0x18, 0x76, 0x65, 0x6e, 0x64, 0x6f, 0x72, 0x5f, 0x72, 0x65, 0x73, 0x65, 0x72, 0x76,
        0x65, 0x64, 0x33, 0x5f, 0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x18
    };
    uint8_t certificationDeclarationTestVector[] = {
        0xd2, 0x84, 0x4b, 0xa2, 0x01, 0x26, 0x04, 0x46, 0x63, 0x73, 0x61, 0x63, 0x64, 0x30, 0xa0, 0x58, 0x1d, 0x15, 0x25,
        0x01, 0x88, 0x99, 0x25, 0x02, 0xfe, 0xff, 0x25, 0x03, 0xd2, 0x04, 0x25, 0x04, 0x2e, 0x16, 0x24, 0x05, 0xaa, 0x25,
        0x06, 0xde, 0xc0, 0x25, 0x07, 0x94, 0x26, 0x18, 0x58, 0x40, 0x96, 0x57, 0x2d, 0xd6, 0x3c, 0x03, 0x64, 0x0b, 0x28,
        0x67, 0x02, 0xbd, 0x6b, 0xba, 0x48, 0xac, 0x7c, 0x83, 0x54, 0x9b, 0x68, 0x73, 0x29, 0x47, 0x48, 0xb9, 0x51, 0xd5,
        0xab, 0x66, 0x62, 0x2e, 0x9d, 0x26, 0x10, 0x41, 0xf8, 0x0e, 0x97, 0x49, 0xfe, 0xff, 0x78, 0x10, 0x02, 0x49, 0x67,
        0xae, 0xdf, 0x41, 0x38, 0x36, 0x5b, 0x0a, 0x22, 0x57, 0x14, 0x9c, 0x9a, 0x12, 0x3e, 0x0d, 0x30, 0xaa
    };
    uint8_t attestationNonceTestVector[]     = { 0xe0, 0x42, 0x1b, 0x91, 0xc6, 0xfd, 0xcd, 0xb4, 0x0e, 0x2a, 0x4d,
                                             0x2c, 0xf3, 0x1d, 0xb2, 0xb4, 0xe1, 0x8b, 0x41, 0x1b, 0x1d, 0x3a,
                                             0xd4, 0xd1, 0x2a, 0x9d, 0x90, 0xaa, 0x8e, 0x52, 0xfa, 0xe2 };
    uint32_t timestampTestVector             = 677103357;
    uint8_t vendorReserved1TestVector[]      = { 0x73, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x5f, 0x76, 0x65, 0x6e, 0x64, 0x6f,
                                            0x72, 0x5f, 0x72, 0x65, 0x73, 0x65, 0x72, 0x76, 0x65, 0x64, 0x31 };
    uint8_t vendorReserved3TestVector[]      = { 0x76, 0x65, 0x6e, 0x64, 0x6f, 0x72, 0x5f, 0x72, 0x65, 0x73, 0x65, 0x72,
                                            0x76, 0x65, 0x64, 0x33, 0x5f, 0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65 };
    ByteSpan vendorReservedArrayTestVector[] = { ByteSpan(vendorReserved1TestVector), ByteSpan(vendorReserved3TestVector) };
    uint16_t vendorIdTestVector              = 0xFFF1;
    uint16_t profileNumTestVector            = 0x003E;

    ByteSpan certificationDeclarationDeconstructed;
    ByteSpan attestationNonceDeconstructed;
    uint32_t timestampDeconstructed;
    ByteSpan firmwareInfoDeconstructed;
    ByteSpan vendorReservedDeconstructed[3];
    size_t vendorReservedDeconstructedSize = ArraySize(vendorReservedDeconstructed);
    uint16_t vendorIdDeconstructed;
    uint16_t profileNumDeconstructed;

    err = DeconstructAttestationElements(ByteSpan(attestationElementsTestVector), certificationDeclarationDeconstructed,
                                         attestationNonceDeconstructed, timestampDeconstructed, firmwareInfoDeconstructed,
                                         vendorReservedDeconstructed, vendorReservedDeconstructedSize, vendorIdDeconstructed,
                                         profileNumDeconstructed);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, certificationDeclarationDeconstructed.data_equal(ByteSpan(certificationDeclarationTestVector)));
    NL_TEST_ASSERT(inSuite, attestationNonceDeconstructed.data_equal(ByteSpan(attestationNonceTestVector)));
    NL_TEST_ASSERT(inSuite, timestampTestVector == timestampDeconstructed);
    NL_TEST_ASSERT(inSuite, firmwareInfoDeconstructed.empty());
    NL_TEST_ASSERT(inSuite, ArraySize(vendorReservedArrayTestVector) == vendorReservedDeconstructedSize);
    for (size_t i = 0; i < ArraySize(vendorReservedArrayTestVector); ++i)
    {
        NL_TEST_ASSERT(inSuite, vendorReservedArrayTestVector[i].data_equal(vendorReservedDeconstructed[i]));
    }
    NL_TEST_ASSERT(inSuite, vendorIdDeconstructed == vendorIdTestVector);
    NL_TEST_ASSERT(inSuite, profileNumDeconstructed == profileNumTestVector);
}

static void TestAttestationElements_DeconstructionWithFirmwareInfo(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    uint8_t attestationElementsTestVectorWithFirmwareInfo[] = {
        0x15, 0x30, 0x01, 0x70, 0xd2, 0x84, 0x4b, 0xa2, 0x01, 0x26, 0x04, 0x46, 0x63, 0x73, 0x61, 0x63, 0x64, 0x30, 0xa0, 0x58,
        0x1d, 0x15, 0x25, 0x01, 0x88, 0x99, 0x25, 0x02, 0xfe, 0xff, 0x25, 0x03, 0xd2, 0x04, 0x25, 0x04, 0x2e, 0x16, 0x24, 0x05,
        0xaa, 0x25, 0x06, 0xde, 0xc0, 0x25, 0x07, 0x94, 0x26, 0x18, 0x58, 0x40, 0x96, 0x57, 0x2d, 0xd6, 0x3c, 0x03, 0x64, 0x0b,
        0x28, 0x67, 0x02, 0xbd, 0x6b, 0xba, 0x48, 0xac, 0x7c, 0x83, 0x54, 0x9b, 0x68, 0x73, 0x29, 0x47, 0x48, 0xb9, 0x51, 0xd5,
        0xab, 0x66, 0x62, 0x2e, 0x9d, 0x26, 0x10, 0x41, 0xf8, 0x0e, 0x97, 0x49, 0xfe, 0xff, 0x78, 0x10, 0x02, 0x49, 0x67, 0xae,
        0xdf, 0x41, 0x38, 0x36, 0x5b, 0x0a, 0x22, 0x57, 0x14, 0x9c, 0x9a, 0x12, 0x3e, 0x0d, 0x30, 0xaa, 0x30, 0x02, 0x20, 0xe0,
        0x42, 0x1b, 0x91, 0xc6, 0xfd, 0xcd, 0xb4, 0x0e, 0x2a, 0x4d, 0x2c, 0xf3, 0x1d, 0xb2, 0xb4, 0xe1, 0x8b, 0x41, 0x1b, 0x1d,
        0x3a, 0xd4, 0xd1, 0x2a, 0x9d, 0x90, 0xaa, 0x8e, 0x52, 0xfa, 0xe2, 0x26, 0x03, 0xfd, 0xc6, 0x5b, 0x28, 0x30, 0x04, 0x69,
        0x30, 0x67, 0x80, 0x01, 0x05, 0xA1, 0x62, 0x30, 0x2F, 0xA0, 0x0B, 0x80, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04,
        0x02, 0x01, 0x81, 0x20, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x30, 0x2F, 0xA0, 0x0B,
        0x80, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x81, 0x20, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
        0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A,
        0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0xd0, 0xf1, 0xff, 0x3e, 0x00, 0x01, 0x00, 0x17, 0x73, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x5f,
        0x76, 0x65, 0x6e, 0x64, 0x6f, 0x72, 0x5f, 0x72, 0x65, 0x73, 0x65, 0x72, 0x76, 0x65, 0x64, 0x31, 0xd0, 0xf1, 0xff, 0x3e,
        0x00, 0x03, 0x00, 0x18, 0x76, 0x65, 0x6e, 0x64, 0x6f, 0x72, 0x5f, 0x72, 0x65, 0x73, 0x65, 0x72, 0x76, 0x65, 0x64, 0x33,
        0x5f, 0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x18
    };
    uint8_t certificationDeclarationTestVector[] = {
        0xd2, 0x84, 0x4b, 0xa2, 0x01, 0x26, 0x04, 0x46, 0x63, 0x73, 0x61, 0x63, 0x64, 0x30, 0xa0, 0x58, 0x1d, 0x15, 0x25,
        0x01, 0x88, 0x99, 0x25, 0x02, 0xfe, 0xff, 0x25, 0x03, 0xd2, 0x04, 0x25, 0x04, 0x2e, 0x16, 0x24, 0x05, 0xaa, 0x25,
        0x06, 0xde, 0xc0, 0x25, 0x07, 0x94, 0x26, 0x18, 0x58, 0x40, 0x96, 0x57, 0x2d, 0xd6, 0x3c, 0x03, 0x64, 0x0b, 0x28,
        0x67, 0x02, 0xbd, 0x6b, 0xba, 0x48, 0xac, 0x7c, 0x83, 0x54, 0x9b, 0x68, 0x73, 0x29, 0x47, 0x48, 0xb9, 0x51, 0xd5,
        0xab, 0x66, 0x62, 0x2e, 0x9d, 0x26, 0x10, 0x41, 0xf8, 0x0e, 0x97, 0x49, 0xfe, 0xff, 0x78, 0x10, 0x02, 0x49, 0x67,
        0xae, 0xdf, 0x41, 0x38, 0x36, 0x5b, 0x0a, 0x22, 0x57, 0x14, 0x9c, 0x9a, 0x12, 0x3e, 0x0d, 0x30, 0xaa
    };
    uint8_t attestationNonceTestVector[] = { 0xe0, 0x42, 0x1b, 0x91, 0xc6, 0xfd, 0xcd, 0xb4, 0x0e, 0x2a, 0x4d,
                                             0x2c, 0xf3, 0x1d, 0xb2, 0xb4, 0xe1, 0x8b, 0x41, 0x1b, 0x1d, 0x3a,
                                             0xd4, 0xd1, 0x2a, 0x9d, 0x90, 0xaa, 0x8e, 0x52, 0xfa, 0xe2 };
    uint32_t timestampTestVector         = 677103357;
    uint8_t firmwareInfoTestVector[] = { 0x30, 0x67, 0x80, 0x01, 0x05, 0xA1, 0x62, 0x30, 0x2F, 0xA0, 0x0B, 0x80, 0x09, 0x60, 0x86,
                                         0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x81, 0x20, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55,
                                         0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11, 0x22, 0x33, 0x44,
                                         0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x30, 0x2F, 0xA0, 0x0B,
                                         0x80, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x81, 0x20, 0x10, 0x11,
                                         0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x10,
                                         0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F };
    uint8_t vendorReserved1TestVector[]      = { 0x73, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x5f, 0x76, 0x65, 0x6e, 0x64, 0x6f,
                                            0x72, 0x5f, 0x72, 0x65, 0x73, 0x65, 0x72, 0x76, 0x65, 0x64, 0x31 };
    uint8_t vendorReserved3TestVector[]      = { 0x76, 0x65, 0x6e, 0x64, 0x6f, 0x72, 0x5f, 0x72, 0x65, 0x73, 0x65, 0x72,
                                            0x76, 0x65, 0x64, 0x33, 0x5f, 0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65 };
    ByteSpan vendorReservedArrayTestVector[] = { ByteSpan(vendorReserved1TestVector), ByteSpan(vendorReserved3TestVector) };
    uint16_t vendorIdTestVector              = 0xFFF1;
    uint16_t profileNumTestVector            = 0x003E;

    ByteSpan certificationDeclarationDeconstructed;
    ByteSpan attestationNonceDeconstructed;
    uint32_t timestampDeconstructed;
    ByteSpan firmwareInfoDeconstructed;
    ByteSpan vendorReservedDeconstructed[3];
    size_t vendorReservedDeconstructedSize = ArraySize(vendorReservedDeconstructed);
    uint16_t vendorIdDeconstructed;
    uint16_t profileNumDeconstructed;

    err = DeconstructAttestationElements(ByteSpan(attestationElementsTestVectorWithFirmwareInfo),
                                         certificationDeclarationDeconstructed, attestationNonceDeconstructed,
                                         timestampDeconstructed, firmwareInfoDeconstructed, vendorReservedDeconstructed,
                                         vendorReservedDeconstructedSize, vendorIdDeconstructed, profileNumDeconstructed);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, certificationDeclarationDeconstructed.data_equal(ByteSpan(certificationDeclarationTestVector)));
    NL_TEST_ASSERT(inSuite, attestationNonceDeconstructed.data_equal(ByteSpan(attestationNonceTestVector)));
    NL_TEST_ASSERT(inSuite, timestampTestVector == timestampDeconstructed);
    NL_TEST_ASSERT(inSuite, firmwareInfoDeconstructed.data_equal(ByteSpan(firmwareInfoTestVector)));
    NL_TEST_ASSERT(inSuite, ArraySize(vendorReservedArrayTestVector) == vendorReservedDeconstructedSize);
    for (size_t i = 0; i < ArraySize(vendorReservedArrayTestVector); ++i)
    {
        NL_TEST_ASSERT(inSuite, vendorReservedArrayTestVector[i].data_equal(vendorReservedDeconstructed[i]));
    }
    NL_TEST_ASSERT(inSuite, vendorIdDeconstructed == vendorIdTestVector);
    NL_TEST_ASSERT(inSuite, profileNumDeconstructed == profileNumTestVector);
}

static void TestAttestationElements_DeconstructionUnordered(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    uint8_t attestationElementsUnorderedTestVector[] = {
        0x15, 0x30, 0x02, 0x20, 0xe0, 0x42, 0x1b, 0x91, 0xc6, 0xfd, 0xcd, 0xb4, 0x0e, 0x2a, 0x4d, 0x2c, 0xf3, 0x1d, 0xb2,
        0xb4, 0xe1, 0x8b, 0x41, 0x1b, 0x1d, 0x3a, 0xd4, 0xd1, 0x2a, 0x9d, 0x90, 0xaa, 0x8e, 0x52, 0xfa, 0xe2, 0x30, 0x01,
        0x70, 0xd2, 0x84, 0x4b, 0xa2, 0x01, 0x26, 0x04, 0x46, 0x63, 0x73, 0x61, 0x63, 0x64, 0x30, 0xa0, 0x58, 0x1d, 0x15,
        0x25, 0x01, 0x88, 0x99, 0x25, 0x02, 0xfe, 0xff, 0x25, 0x03, 0xd2, 0x04, 0x25, 0x04, 0x2e, 0x16, 0x24, 0x05, 0xaa,
        0x25, 0x06, 0xde, 0xc0, 0x25, 0x07, 0x94, 0x26, 0x18, 0x58, 0x40, 0x96, 0x57, 0x2d, 0xd6, 0x3c, 0x03, 0x64, 0x0b,
        0x28, 0x67, 0x02, 0xbd, 0x6b, 0xba, 0x48, 0xac, 0x7c, 0x83, 0x54, 0x9b, 0x68, 0x73, 0x29, 0x47, 0x48, 0xb9, 0x51,
        0xd5, 0xab, 0x66, 0x62, 0x2e, 0x9d, 0x26, 0x10, 0x41, 0xf8, 0x0e, 0x97, 0x49, 0xfe, 0xff, 0x78, 0x10, 0x02, 0x49,
        0x67, 0xae, 0xdf, 0x41, 0x38, 0x36, 0x5b, 0x0a, 0x22, 0x57, 0x14, 0x9c, 0x9a, 0x12, 0x3e, 0x0d, 0x30, 0xaa, 0x26,
        0x03, 0xfd, 0xc6, 0x5b, 0x28, 0xd0, 0xf1, 0xff, 0x3e, 0x00, 0x01, 0x00, 0x17, 0x73, 0x61, 0x6d, 0x70, 0x6c, 0x65,
        0x5f, 0x76, 0x65, 0x6e, 0x64, 0x6f, 0x72, 0x5f, 0x72, 0x65, 0x73, 0x65, 0x72, 0x76, 0x65, 0x64, 0x31, 0xd0, 0xf1,
        0xff, 0x3e, 0x00, 0x03, 0x00, 0x18, 0x76, 0x65, 0x6e, 0x64, 0x6f, 0x72, 0x5f, 0x72, 0x65, 0x73, 0x65, 0x72, 0x76,
        0x65, 0x64, 0x33, 0x5f, 0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x18
    };

    ByteSpan certificationDeclarationDeconstructed;
    ByteSpan attestationNonceDeconstructed;
    uint32_t timestampDeconstructed;
    ByteSpan firmwareInfoDeconstructed;
    ByteSpan vendorReservedDeconstructed[3];
    size_t vendorReservedDeconstructedSize = ArraySize(vendorReservedDeconstructed);
    uint16_t vendorIdDeconstructed;
    uint16_t profileNumDeconstructed;

    err = DeconstructAttestationElements(ByteSpan(attestationElementsUnorderedTestVector), certificationDeclarationDeconstructed,
                                         attestationNonceDeconstructed, timestampDeconstructed, firmwareInfoDeconstructed,
                                         vendorReservedDeconstructed, vendorReservedDeconstructedSize, vendorIdDeconstructed,
                                         profileNumDeconstructed);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_IM_MALFORMED_COMMAND_DATA_ELEMENT);
}

/**
 *   Test Suite. It lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] = {
    NL_TEST_DEF("Test Device Attestation Elements Roundtrip", TestAttestationElements_Roundtrip),
    NL_TEST_DEF("Test Device Attestation Elements Construction", TestAttestationElements_Construction),
    NL_TEST_DEF("Test Device Attestation Elements Deconstruction", TestAttestationElements_Deconstruction),
    NL_TEST_DEF("Test Device Attestation Elements Deconstruction with Firmware Information", TestAttestationElements_DeconstructionWithFirmwareInfo),
    NL_TEST_DEF("Test Device Attestation Elements Deconstruction - Corrupted/Out of Order TLV", TestAttestationElements_DeconstructionUnordered),
    NL_TEST_SENTINEL()
};
// clang-format on

/**
 *  Set up the test suite.
 */
int TestDeviceAttestationElementsConstruction_Setup(void * inContext)
{
    CHIP_ERROR error = chip::Platform::MemoryInit();
    if (error != CHIP_NO_ERROR)
        return FAILURE;
    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
int TestDeviceAttestationElementsConstruction_Teardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

int TestDeviceAttestationElementsConstruction()
{
    // clang-format off
    nlTestSuite theSuite =
    {
        "Device Attestation Elements Construction",
        &sTests[0],
        TestDeviceAttestationElementsConstruction_Setup,
        TestDeviceAttestationElementsConstruction_Teardown
    };
    // clang-format on
    nlTestRunner(&theSuite, nullptr);
    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestDeviceAttestationElementsConstruction);
