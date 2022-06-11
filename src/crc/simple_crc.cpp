/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017, 2020, 2022 Daniel Jerolm

    This file is part of the General Purpose Class Collection (GPCC).

    The General Purpose Class Collection (GPCC) is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The General Purpose Class Collection (GPCC) is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.

                                      ---

    A special exception to the GPL can be applied should you wish to distribute
    a combined work that includes the General Purpose Class Collection (GPCC), without being obliged
    to provide the source code for any proprietary components. See the file
    license_exception.txt for full details of how and when the exception can be applied.
*/

#include "simple_crc.hpp"
#include "gpcc/src/Compiler/builtins.hpp"

namespace gpcc {
namespace crc  {

/**
 * \ingroup GPCC_CRC
 * \brief CRC LUT (normal) for calculating CRC-32A (BZIP2) and CRC-32B (Ethernet, IEEE 802.3) checksums.
 *
 * See also: @ref crc32ab_table_reflected
 *
 * Polynomial: Forward\n
 * X^32+X^26+X^23+X^22+X^16+X^12+X^11+X^10+X^8+X^7+X^5+X^4+X^2+X^1+1 -> 0x04C11DB7
 *
 * Hamming distance | Payload size bit (byte)
 * ---------------- | -----------------------
 * 4                | up to 91607 (11450)
 * 5                | up to 2974 (371)
 * 6                | up to 268 (33)
 *
 * This LUT can be used to calculate the following types of CRC:
 *
 * CRC Name           | Start Value | CRC shift | Bit-reverse data | Bit-reverse final CRC | XOR final CRC | CRC appended to data... | Receiver magic value
 * ------------------ | ----------- | --------- | ---------------- | --------------------- | ------------- | ----------------------- | --------------------
 * CRC32-A (BZIP2)    | 0xFFFFFFFF  | left      | no               | no                    | 0xFFFFFFFF    | High-byte first         | 0x38FB2284
 * CRC32-B (Ethernet) | 0xFFFFFFFF  | left      | yes              | yes                   | 0xFFFFFFFF    | Low-byte first          | 0x2144DF1C
 *
 */
uint32_t const crc32ab_table_normal[256] =
{
  0x00000000UL, 0x04C11DB7UL, 0x09823B6EUL, 0x0D4326D9UL, 0x130476DCUL, 0x17C56B6BUL, 0x1A864DB2UL, 0x1E475005UL,
  0x2608EDB8UL, 0x22C9F00FUL, 0x2F8AD6D6UL, 0x2B4BCB61UL, 0x350C9B64UL, 0x31CD86D3UL, 0x3C8EA00AUL, 0x384FBDBDUL,
  0x4C11DB70UL, 0x48D0C6C7UL, 0x4593E01EUL, 0x4152FDA9UL, 0x5F15ADACUL, 0x5BD4B01BUL, 0x569796C2UL, 0x52568B75UL,
  0x6A1936C8UL, 0x6ED82B7FUL, 0x639B0DA6UL, 0x675A1011UL, 0x791D4014UL, 0x7DDC5DA3UL, 0x709F7B7AUL, 0x745E66CDUL,
  0x9823B6E0UL, 0x9CE2AB57UL, 0x91A18D8EUL, 0x95609039UL, 0x8B27C03CUL, 0x8FE6DD8BUL, 0x82A5FB52UL, 0x8664E6E5UL,
  0xBE2B5B58UL, 0xBAEA46EFUL, 0xB7A96036UL, 0xB3687D81UL, 0xAD2F2D84UL, 0xA9EE3033UL, 0xA4AD16EAUL, 0xA06C0B5DUL,
  0xD4326D90UL, 0xD0F37027UL, 0xDDB056FEUL, 0xD9714B49UL, 0xC7361B4CUL, 0xC3F706FBUL, 0xCEB42022UL, 0xCA753D95UL,
  0xF23A8028UL, 0xF6FB9D9FUL, 0xFBB8BB46UL, 0xFF79A6F1UL, 0xE13EF6F4UL, 0xE5FFEB43UL, 0xE8BCCD9AUL, 0xEC7DD02DUL,
  0x34867077UL, 0x30476DC0UL, 0x3D044B19UL, 0x39C556AEUL, 0x278206ABUL, 0x23431B1CUL, 0x2E003DC5UL, 0x2AC12072UL,
  0x128E9DCFUL, 0x164F8078UL, 0x1B0CA6A1UL, 0x1FCDBB16UL, 0x018AEB13UL, 0x054BF6A4UL, 0x0808D07DUL, 0x0CC9CDCAUL,
  0x7897AB07UL, 0x7C56B6B0UL, 0x71159069UL, 0x75D48DDEUL, 0x6B93DDDBUL, 0x6F52C06CUL, 0x6211E6B5UL, 0x66D0FB02UL,
  0x5E9F46BFUL, 0x5A5E5B08UL, 0x571D7DD1UL, 0x53DC6066UL, 0x4D9B3063UL, 0x495A2DD4UL, 0x44190B0DUL, 0x40D816BAUL,
  0xACA5C697UL, 0xA864DB20UL, 0xA527FDF9UL, 0xA1E6E04EUL, 0xBFA1B04BUL, 0xBB60ADFCUL, 0xB6238B25UL, 0xB2E29692UL,
  0x8AAD2B2FUL, 0x8E6C3698UL, 0x832F1041UL, 0x87EE0DF6UL, 0x99A95DF3UL, 0x9D684044UL, 0x902B669DUL, 0x94EA7B2AUL,
  0xE0B41DE7UL, 0xE4750050UL, 0xE9362689UL, 0xEDF73B3EUL, 0xF3B06B3BUL, 0xF771768CUL, 0xFA325055UL, 0xFEF34DE2UL,
  0xC6BCF05FUL, 0xC27DEDE8UL, 0xCF3ECB31UL, 0xCBFFD686UL, 0xD5B88683UL, 0xD1799B34UL, 0xDC3ABDEDUL, 0xD8FBA05AUL,
  0x690CE0EEUL, 0x6DCDFD59UL, 0x608EDB80UL, 0x644FC637UL, 0x7A089632UL, 0x7EC98B85UL, 0x738AAD5CUL, 0x774BB0EBUL,
  0x4F040D56UL, 0x4BC510E1UL, 0x46863638UL, 0x42472B8FUL, 0x5C007B8AUL, 0x58C1663DUL, 0x558240E4UL, 0x51435D53UL,
  0x251D3B9EUL, 0x21DC2629UL, 0x2C9F00F0UL, 0x285E1D47UL, 0x36194D42UL, 0x32D850F5UL, 0x3F9B762CUL, 0x3B5A6B9BUL,
  0x0315D626UL, 0x07D4CB91UL, 0x0A97ED48UL, 0x0E56F0FFUL, 0x1011A0FAUL, 0x14D0BD4DUL, 0x19939B94UL, 0x1D528623UL,
  0xF12F560EUL, 0xF5EE4BB9UL, 0xF8AD6D60UL, 0xFC6C70D7UL, 0xE22B20D2UL, 0xE6EA3D65UL, 0xEBA91BBCUL, 0xEF68060BUL,
  0xD727BBB6UL, 0xD3E6A601UL, 0xDEA580D8UL, 0xDA649D6FUL, 0xC423CD6AUL, 0xC0E2D0DDUL, 0xCDA1F604UL, 0xC960EBB3UL,
  0xBD3E8D7EUL, 0xB9FF90C9UL, 0xB4BCB610UL, 0xB07DABA7UL, 0xAE3AFBA2UL, 0xAAFBE615UL, 0xA7B8C0CCUL, 0xA379DD7BUL,
  0x9B3660C6UL, 0x9FF77D71UL, 0x92B45BA8UL, 0x9675461FUL, 0x8832161AUL, 0x8CF30BADUL, 0x81B02D74UL, 0x857130C3UL,
  0x5D8A9099UL, 0x594B8D2EUL, 0x5408ABF7UL, 0x50C9B640UL, 0x4E8EE645UL, 0x4A4FFBF2UL, 0x470CDD2BUL, 0x43CDC09CUL,
  0x7B827D21UL, 0x7F436096UL, 0x7200464FUL, 0x76C15BF8UL, 0x68860BFDUL, 0x6C47164AUL, 0x61043093UL, 0x65C52D24UL,
  0x119B4BE9UL, 0x155A565EUL, 0x18197087UL, 0x1CD86D30UL, 0x029F3D35UL, 0x065E2082UL, 0x0B1D065BUL, 0x0FDC1BECUL,
  0x3793A651UL, 0x3352BBE6UL, 0x3E119D3FUL, 0x3AD08088UL, 0x2497D08DUL, 0x2056CD3AUL, 0x2D15EBE3UL, 0x29D4F654UL,
  0xC5A92679UL, 0xC1683BCEUL, 0xCC2B1D17UL, 0xC8EA00A0UL, 0xD6AD50A5UL, 0xD26C4D12UL, 0xDF2F6BCBUL, 0xDBEE767CUL,
  0xE3A1CBC1UL, 0xE760D676UL, 0xEA23F0AFUL, 0xEEE2ED18UL, 0xF0A5BD1DUL, 0xF464A0AAUL, 0xF9278673UL, 0xFDE69BC4UL,
  0x89B8FD09UL, 0x8D79E0BEUL, 0x803AC667UL, 0x84FBDBD0UL, 0x9ABC8BD5UL, 0x9E7D9662UL, 0x933EB0BBUL, 0x97FFAD0CUL,
  0xAFB010B1UL, 0xAB710D06UL, 0xA6322BDFUL, 0xA2F33668UL, 0xBCB4666DUL, 0xB8757BDAUL, 0xB5365D03UL, 0xB1F740B4UL
};

/**
 * \ingroup GPCC_CRC
 * \brief CRC LUT (reflected) for calculating CRC-32A (BZIP2) and CRC-32B (Ethernet, IEEE 802.3) checksums.
 *
 * See also: @ref crc32ab_table_normal
 *
 * Polynomial: Reverse\n
 * X^32+X^26+X^23+X^22+X^16+X^12+X^11+X^10+X^8+X^7+X^5+X^4+X^2+X^1+1 -> 0xEDB88320
 *
 * Hamming distance | Payload size bit (byte)
 * ---------------- | -----------------------
 * 4                | up to 91607 (11450)
 * 5                | up to 2974 (371)
 * 6                | up to 268 (33)
 *
 * This LUT can be used to calculate the following types of CRC:
 *
 * CRC Name           | Start value | CRC shift | Bit-reverse data | Bit-reverse final CRC | XOR final CRC | CRC appended to data... | Receiver magic value
 * ------------------ | ----------- | --------- | ---------------- | --------------------- | ------------- | ----------------------- | --------------------
 * CRC32-A (BZIP2)    | 0xFFFFFFFF  | right     | yes              | yes                   | 0xFFFFFFFF    | High-byte first         | 0x38FB2284
 * CRC32-B (Ethernet) | 0xFFFFFFFF  | right     | no               | no                    | 0xFFFFFFFF    | Low-byte first          | 0x2144DF1C
 *
 */
uint32_t const crc32ab_table_reflected[256] =
{
  0x00000000UL, 0x77073096UL, 0xEE0E612CUL, 0x990951BAUL, 0x076DC419UL, 0x706AF48FUL, 0xE963A535UL, 0x9E6495A3UL,
  0x0EDB8832UL, 0x79DCB8A4UL, 0xE0D5E91EUL, 0x97D2D988UL, 0x09B64C2BUL, 0x7EB17CBDUL, 0xE7B82D07UL, 0x90BF1D91UL,
  0x1DB71064UL, 0x6AB020F2UL, 0xF3B97148UL, 0x84BE41DEUL, 0x1ADAD47DUL, 0x6DDDE4EBUL, 0xF4D4B551UL, 0x83D385C7UL,
  0x136C9856UL, 0x646BA8C0UL, 0xFD62F97AUL, 0x8A65C9ECUL, 0x14015C4FUL, 0x63066CD9UL, 0xFA0F3D63UL, 0x8D080DF5UL,
  0x3B6E20C8UL, 0x4C69105EUL, 0xD56041E4UL, 0xA2677172UL, 0x3C03E4D1UL, 0x4B04D447UL, 0xD20D85FDUL, 0xA50AB56BUL,
  0x35B5A8FAUL, 0x42B2986CUL, 0xDBBBC9D6UL, 0xACBCF940UL, 0x32D86CE3UL, 0x45DF5C75UL, 0xDCD60DCFUL, 0xABD13D59UL,
  0x26D930ACUL, 0x51DE003AUL, 0xC8D75180UL, 0xBFD06116UL, 0x21B4F4B5UL, 0x56B3C423UL, 0xCFBA9599UL, 0xB8BDA50FUL,
  0x2802B89EUL, 0x5F058808UL, 0xC60CD9B2UL, 0xB10BE924UL, 0x2F6F7C87UL, 0x58684C11UL, 0xC1611DABUL, 0xB6662D3DUL,
  0x76DC4190UL, 0x01DB7106UL, 0x98D220BCUL, 0xEFD5102AUL, 0x71B18589UL, 0x06B6B51FUL, 0x9FBFE4A5UL, 0xE8B8D433UL,
  0x7807C9A2UL, 0x0F00F934UL, 0x9609A88EUL, 0xE10E9818UL, 0x7F6A0DBBUL, 0x086D3D2DUL, 0x91646C97UL, 0xE6635C01UL,
  0x6B6B51F4UL, 0x1C6C6162UL, 0x856530D8UL, 0xF262004EUL, 0x6C0695EDUL, 0x1B01A57BUL, 0x8208F4C1UL, 0xF50FC457UL,
  0x65B0D9C6UL, 0x12B7E950UL, 0x8BBEB8EAUL, 0xFCB9887CUL, 0x62DD1DDFUL, 0x15DA2D49UL, 0x8CD37CF3UL, 0xFBD44C65UL,
  0x4DB26158UL, 0x3AB551CEUL, 0xA3BC0074UL, 0xD4BB30E2UL, 0x4ADFA541UL, 0x3DD895D7UL, 0xA4D1C46DUL, 0xD3D6F4FBUL,
  0x4369E96AUL, 0x346ED9FCUL, 0xAD678846UL, 0xDA60B8D0UL, 0x44042D73UL, 0x33031DE5UL, 0xAA0A4C5FUL, 0xDD0D7CC9UL,
  0x5005713CUL, 0x270241AAUL, 0xBE0B1010UL, 0xC90C2086UL, 0x5768B525UL, 0x206F85B3UL, 0xB966D409UL, 0xCE61E49FUL,
  0x5EDEF90EUL, 0x29D9C998UL, 0xB0D09822UL, 0xC7D7A8B4UL, 0x59B33D17UL, 0x2EB40D81UL, 0xB7BD5C3BUL, 0xC0BA6CADUL,
  0xEDB88320UL, 0x9ABFB3B6UL, 0x03B6E20CUL, 0x74B1D29AUL, 0xEAD54739UL, 0x9DD277AFUL, 0x04DB2615UL, 0x73DC1683UL,
  0xE3630B12UL, 0x94643B84UL, 0x0D6D6A3EUL, 0x7A6A5AA8UL, 0xE40ECF0BUL, 0x9309FF9DUL, 0x0A00AE27UL, 0x7D079EB1UL,
  0xF00F9344UL, 0x8708A3D2UL, 0x1E01F268UL, 0x6906C2FEUL, 0xF762575DUL, 0x806567CBUL, 0x196C3671UL, 0x6E6B06E7UL,
  0xFED41B76UL, 0x89D32BE0UL, 0x10DA7A5AUL, 0x67DD4ACCUL, 0xF9B9DF6FUL, 0x8EBEEFF9UL, 0x17B7BE43UL, 0x60B08ED5UL,
  0xD6D6A3E8UL, 0xA1D1937EUL, 0x38D8C2C4UL, 0x4FDFF252UL, 0xD1BB67F1UL, 0xA6BC5767UL, 0x3FB506DDUL, 0x48B2364BUL,
  0xD80D2BDAUL, 0xAF0A1B4CUL, 0x36034AF6UL, 0x41047A60UL, 0xDF60EFC3UL, 0xA867DF55UL, 0x316E8EEFUL, 0x4669BE79UL,
  0xCB61B38CUL, 0xBC66831AUL, 0x256FD2A0UL, 0x5268E236UL, 0xCC0C7795UL, 0xBB0B4703UL, 0x220216B9UL, 0x5505262FUL,
  0xC5BA3BBEUL, 0xB2BD0B28UL, 0x2BB45A92UL, 0x5CB36A04UL, 0xC2D7FFA7UL, 0xB5D0CF31UL, 0x2CD99E8BUL, 0x5BDEAE1DUL,
  0x9B64C2B0UL, 0xEC63F226UL, 0x756AA39CUL, 0x026D930AUL, 0x9C0906A9UL, 0xEB0E363FUL, 0x72076785UL, 0x05005713UL,
  0x95BF4A82UL, 0xE2B87A14UL, 0x7BB12BAEUL, 0x0CB61B38UL, 0x92D28E9BUL, 0xE5D5BE0DUL, 0x7CDCEFB7UL, 0x0BDBDF21UL,
  0x86D3D2D4UL, 0xF1D4E242UL, 0x68DDB3F8UL, 0x1FDA836EUL, 0x81BE16CDUL, 0xF6B9265BUL, 0x6FB077E1UL, 0x18B74777UL,
  0x88085AE6UL, 0xFF0F6A70UL, 0x66063BCAUL, 0x11010B5CUL, 0x8F659EFFUL, 0xF862AE69UL, 0x616BFFD3UL, 0x166CCF45UL,
  0xA00AE278UL, 0xD70DD2EEUL, 0x4E048354UL, 0x3903B3C2UL, 0xA7672661UL, 0xD06016F7UL, 0x4969474DUL, 0x3E6E77DBUL,
  0xAED16A4AUL, 0xD9D65ADCUL, 0x40DF0B66UL, 0x37D83BF0UL, 0xA9BCAE53UL, 0xDEBB9EC5UL, 0x47B2CF7FUL, 0x30B5FFE9UL,
  0xBDBDF21CUL, 0xCABAC28AUL, 0x53B39330UL, 0x24B4A3A6UL, 0xBAD03605UL, 0xCDD70693UL, 0x54DE5729UL, 0x23D967BFUL,
  0xB3667A2EUL, 0xC4614AB8UL, 0x5D681B02UL, 0x2A6F2B94UL, 0xB40BBE37UL, 0xC30C8EA1UL, 0x5A05DF1BUL, 0x2D02EF8DUL
};

/**
 * \ingroup GPCC_CRC
 * \brief CRC LUT (normal) for calculating several kinds of CRC-16 checksums (XMODEM, CCITT FALSE, ...).
 *
 * Polynomial: Forward\n
 * X^16+X^12+X^5+1 -> 0x1021
 *
 * Hamming distance | Payload size bit (byte)
 * ---------------- | -----------------------
 * 4                | up to 32767 (4095)
 *
 * This LUT can be used to calculate the following types of CRC:
 *
 * CRC Name             | Start value | CRC shift | Bit-reverse data | Bit-reverse final CRC | XOR final CRC | CRC appended to data... | Receiver magic value
 * -------------------- | ----------- | --------- | ---------------- | --------------------- | ------------- | ----------------------- | --------------------
 * CRC-16 (XMODEM)      | 0x0000      | left      | no               | no                    | 0x0000        | High-byte first         | 0x0000
 * CRC-16 (CCITT FALSE) | 0xFFFF      | left      | no               | no                    | 0x0000        | High-byte first         | 0x0000
 *
 * There are a lot more types of CRC which use this table than those shown above.
 *
 */
uint16_t const crc16_ccitt_table_normal[256] =
{
  0x0000U, 0x1021U, 0x2042U, 0x3063U, 0x4084U, 0x50a5U, 0x60c6U, 0x70e7U,
  0x8108U, 0x9129U, 0xa14aU, 0xb16bU, 0xc18cU, 0xd1adU, 0xe1ceU, 0xf1efU,
  0x1231U, 0x0210U, 0x3273U, 0x2252U, 0x52b5U, 0x4294U, 0x72f7U, 0x62d6U,
  0x9339U, 0x8318U, 0xb37bU, 0xa35aU, 0xd3bdU, 0xc39cU, 0xf3ffU, 0xe3deU,
  0x2462U, 0x3443U, 0x0420U, 0x1401U, 0x64e6U, 0x74c7U, 0x44a4U, 0x5485U,
  0xa56aU, 0xb54bU, 0x8528U, 0x9509U, 0xe5eeU, 0xf5cfU, 0xc5acU, 0xd58dU,
  0x3653U, 0x2672U, 0x1611U, 0x0630U, 0x76d7U, 0x66f6U, 0x5695U, 0x46b4U,
  0xb75bU, 0xa77aU, 0x9719U, 0x8738U, 0xf7dfU, 0xe7feU, 0xd79dU, 0xc7bcU,
  0x48c4U, 0x58e5U, 0x6886U, 0x78a7U, 0x0840U, 0x1861U, 0x2802U, 0x3823U,
  0xc9ccU, 0xd9edU, 0xe98eU, 0xf9afU, 0x8948U, 0x9969U, 0xa90aU, 0xb92bU,
  0x5af5U, 0x4ad4U, 0x7ab7U, 0x6a96U, 0x1a71U, 0x0a50U, 0x3a33U, 0x2a12U,
  0xdbfdU, 0xcbdcU, 0xfbbfU, 0xeb9eU, 0x9b79U, 0x8b58U, 0xbb3bU, 0xab1aU,
  0x6ca6U, 0x7c87U, 0x4ce4U, 0x5cc5U, 0x2c22U, 0x3c03U, 0x0c60U, 0x1c41U,
  0xedaeU, 0xfd8fU, 0xcdecU, 0xddcdU, 0xad2aU, 0xbd0bU, 0x8d68U, 0x9d49U,
  0x7e97U, 0x6eb6U, 0x5ed5U, 0x4ef4U, 0x3e13U, 0x2e32U, 0x1e51U, 0x0e70U,
  0xff9fU, 0xefbeU, 0xdfddU, 0xcffcU, 0xbf1bU, 0xaf3aU, 0x9f59U, 0x8f78U,
  0x9188U, 0x81a9U, 0xb1caU, 0xa1ebU, 0xd10cU, 0xc12dU, 0xf14eU, 0xe16fU,
  0x1080U, 0x00a1U, 0x30c2U, 0x20e3U, 0x5004U, 0x4025U, 0x7046U, 0x6067U,
  0x83b9U, 0x9398U, 0xa3fbU, 0xb3daU, 0xc33dU, 0xd31cU, 0xe37fU, 0xf35eU,
  0x02b1U, 0x1290U, 0x22f3U, 0x32d2U, 0x4235U, 0x5214U, 0x6277U, 0x7256U,
  0xb5eaU, 0xa5cbU, 0x95a8U, 0x8589U, 0xf56eU, 0xe54fU, 0xd52cU, 0xc50dU,
  0x34e2U, 0x24c3U, 0x14a0U, 0x0481U, 0x7466U, 0x6447U, 0x5424U, 0x4405U,
  0xa7dbU, 0xb7faU, 0x8799U, 0x97b8U, 0xe75fU, 0xf77eU, 0xc71dU, 0xd73cU,
  0x26d3U, 0x36f2U, 0x0691U, 0x16b0U, 0x6657U, 0x7676U, 0x4615U, 0x5634U,
  0xd94cU, 0xc96dU, 0xf90eU, 0xe92fU, 0x99c8U, 0x89e9U, 0xb98aU, 0xa9abU,
  0x5844U, 0x4865U, 0x7806U, 0x6827U, 0x18c0U, 0x08e1U, 0x3882U, 0x28a3U,
  0xcb7dU, 0xdb5cU, 0xeb3fU, 0xfb1eU, 0x8bf9U, 0x9bd8U, 0xabbbU, 0xbb9aU,
  0x4a75U, 0x5a54U, 0x6a37U, 0x7a16U, 0x0af1U, 0x1ad0U, 0x2ab3U, 0x3a92U,
  0xfd2eU, 0xed0fU, 0xdd6cU, 0xcd4dU, 0xbdaaU, 0xad8bU, 0x9de8U, 0x8dc9U,
  0x7c26U, 0x6c07U, 0x5c64U, 0x4c45U, 0x3ca2U, 0x2c83U, 0x1ce0U, 0x0cc1U,
  0xef1fU, 0xff3eU, 0xcf5dU, 0xdf7cU, 0xaf9bU, 0xbfbaU, 0x8fd9U, 0x9ff8U,
  0x6e17U, 0x7e36U, 0x4e55U, 0x5e74U, 0x2e93U, 0x3eb2U, 0x0ed1U, 0x1ef0U
};

/**
 * \ingroup GPCC_CRC
 * \brief CRC LUT (normal) for calculating several kinds of CRC-8 checksums (CRC8/ITU).
 *
 * Polynomial: Forward\n
 * X^8+X^2+X^1+1 -> 0x07
 *
 * Hamming distance | Payload size bit (byte)
 * ---------------- | -----------------------
 * 4                | up to 127 (15)
 *
 * This LUT can be used to calculate the following types of CRC:
 *
 * CRC Name          | Start value | CRC shift | Bit-reverse data | Bit-reverse final CRC | XOR final CRC | Receiver magic value
 * ----------------- | ----------- | --------- | ---------------- | --------------------- | ------------- | --------------------
 * CRC-8 (ITU-T)     | 0x00        | left      | no               | no                    | 0x55          | 0xF9
 *
 */
uint8_t const crc8_ccitt_table_normal[256] =
{
  0x00U, 0x07U, 0x0eU, 0x09U, 0x1cU, 0x1bU, 0x12U, 0x15U,
  0x38U, 0x3fU, 0x36U, 0x31U, 0x24U, 0x23U, 0x2aU, 0x2dU,
  0x70U, 0x77U, 0x7eU, 0x79U, 0x6cU, 0x6bU, 0x62U, 0x65U,
  0x48U, 0x4fU, 0x46U, 0x41U, 0x54U, 0x53U, 0x5aU, 0x5dU,
  0xe0U, 0xe7U, 0xeeU, 0xe9U, 0xfcU, 0xfbU, 0xf2U, 0xf5U,
  0xd8U, 0xdfU, 0xd6U, 0xd1U, 0xc4U, 0xc3U, 0xcaU, 0xcdU,
  0x90U, 0x97U, 0x9eU, 0x99U, 0x8cU, 0x8bU, 0x82U, 0x85U,
  0xa8U, 0xafU, 0xa6U, 0xa1U, 0xb4U, 0xb3U, 0xbaU, 0xbdU,
  0xc7U, 0xc0U, 0xc9U, 0xceU, 0xdbU, 0xdcU, 0xd5U, 0xd2U,
  0xffU, 0xf8U, 0xf1U, 0xf6U, 0xe3U, 0xe4U, 0xedU, 0xeaU,
  0xb7U, 0xb0U, 0xb9U, 0xbeU, 0xabU, 0xacU, 0xa5U, 0xa2U,
  0x8fU, 0x88U, 0x81U, 0x86U, 0x93U, 0x94U, 0x9dU, 0x9aU,
  0x27U, 0x20U, 0x29U, 0x2eU, 0x3bU, 0x3cU, 0x35U, 0x32U,
  0x1fU, 0x18U, 0x11U, 0x16U, 0x03U, 0x04U, 0x0dU, 0x0aU,
  0x57U, 0x50U, 0x59U, 0x5eU, 0x4bU, 0x4cU, 0x45U, 0x42U,
  0x6fU, 0x68U, 0x61U, 0x66U, 0x73U, 0x74U, 0x7dU, 0x7aU,
  0x89U, 0x8eU, 0x87U, 0x80U, 0x95U, 0x92U, 0x9bU, 0x9cU,
  0xb1U, 0xb6U, 0xbfU, 0xb8U, 0xadU, 0xaaU, 0xa3U, 0xa4U,
  0xf9U, 0xfeU, 0xf7U, 0xf0U, 0xe5U, 0xe2U, 0xebU, 0xecU,
  0xc1U, 0xc6U, 0xcfU, 0xc8U, 0xddU, 0xdaU, 0xd3U, 0xd4U,
  0x69U, 0x6eU, 0x67U, 0x60U, 0x75U, 0x72U, 0x7bU, 0x7cU,
  0x51U, 0x56U, 0x5fU, 0x58U, 0x4dU, 0x4aU, 0x43U, 0x44U,
  0x19U, 0x1eU, 0x17U, 0x10U, 0x05U, 0x02U, 0x0bU, 0x0cU,
  0x21U, 0x26U, 0x2fU, 0x28U, 0x3dU, 0x3aU, 0x33U, 0x34U,
  0x4eU, 0x49U, 0x40U, 0x47U, 0x52U, 0x55U, 0x5cU, 0x5bU,
  0x76U, 0x71U, 0x78U, 0x7fU, 0x6aU, 0x6dU, 0x64U, 0x63U,
  0x3eU, 0x39U, 0x30U, 0x37U, 0x22U, 0x25U, 0x2cU, 0x2bU,
  0x06U, 0x01U, 0x08U, 0x0fU, 0x1aU, 0x1dU, 0x14U, 0x13U,
  0xaeU, 0xa9U, 0xa0U, 0xa7U, 0xb2U, 0xb5U, 0xbcU, 0xbbU,
  0x96U, 0x91U, 0x98U, 0x9fU, 0x8aU, 0x8dU, 0x84U, 0x83U,
  0xdeU, 0xd9U, 0xd0U, 0xd7U, 0xc2U, 0xc5U, 0xccU, 0xcbU,
  0xe6U, 0xe1U, 0xe8U, 0xefU, 0xfaU, 0xfdU, 0xf4U, 0xf3U
};

/**
 * \ingroup GPCC_CRC
 * \brief Generates a LUT for an 8-bit CRC (normal form).
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param forward_polynomial
 * 8-bit polynomial (forward, _bits not reversed_) for which the LUT shall be created.\n
 * Example: X^8+X^5+X^4+1 -> 0x31
 *
 * \param table
 * The LUT is written into this.
 */
void GenerateCRC8Table_normal(uint8_t const forward_polynomial, uint8_t table[256]) noexcept
{
  for (uint_fast16_t i = 0U; i < 256U; i++)
  {
    uint_fast8_t crc = i;

    for (uint_fast8_t j = 0U; j < 8U; j++)
    {
      if ((crc & 0x80U) != 0U)
        crc = (crc << 1U) ^ forward_polynomial;
      else
        crc <<= 1U;
    }

    table[i] = crc;
  }
}

/**
 * \ingroup GPCC_CRC
 * \brief Generates a LUT for an 8-bit CRC (reflected form).
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param reverse_polynomial
 * 8-bit polynomial (_bits reversed_) for which the LUT shall be created.\n
 * Example: X^8+X^5+X^4+1 -> 0x8C
 *
 * \param table
 * The LUT is written into this.
 */
void GenerateCRC8Table_reflected(uint8_t const reverse_polynomial, uint8_t table[256]) noexcept
{
  for (uint_fast16_t i = 0U; i < 256U; i++)
  {
    uint_fast8_t crc = i;

    for (uint_fast8_t j = 0U; j < 8U; j++)
    {
      if ((crc & 0x1U) != 0U)
        crc = (crc >> 1U) ^ reverse_polynomial;
      else
        crc >>= 1U;
    }

    table[i] = crc;
  }
}

/**
 * \ingroup GPCC_CRC
 * \brief Generates a LUT for a 16-bit CRC (normal form).
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param forward_polynomial
 * 16-bit polynomial (forward, _bits not reversed_) for which the LUT shall be created.\n
 * Example: X^16+X^15+X^2+1 -> 0x8005
 *
 * \param table
 * The LUT is written into this.
 */
void GenerateCRC16Table_normal(uint16_t const forward_polynomial, uint16_t table[256]) noexcept
{
  for (uint_fast16_t i = 0U; i < 256U; i++)
  {
    uint_fast16_t crc = i << 8U;

    for (uint_fast8_t j = 0U; j < 8U; j++)
    {
      if ((crc & 0x8000UL) != 0U)
        crc = (crc << 1U) ^ forward_polynomial;
      else
        crc <<= 1U;
    }

    table[i] = crc;
  }
}

/**
 * \ingroup GPCC_CRC
 * \brief Generates a LUT for a 16-bit CRC (reflected form).
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param reverse_polynomial
 * 16-bit polynomial (_bits reversed_) for which the LUT shall be created.\n
 * Example: X^16+X^15+X^2+1 -> 0xA001
 *
 * \param table
 * The LUT is written into this.
 */
void GenerateCRC16Table_reflected(uint16_t const reverse_polynomial, uint16_t table[256]) noexcept
{
  for (uint_fast16_t i = 0U; i < 256U; i++)
  {
    uint_fast16_t crc = i;

    for (uint_fast8_t j = 0U; j < 8U; j++)
    {
      if ((crc & 0x1UL) != 0U)
        crc = (crc >> 1U) ^ reverse_polynomial;
      else
        crc >>= 1U;
    }

    table[i] = crc;
  }
}

/**
 * \ingroup GPCC_CRC
 * \brief Generates a LUT for a 32-bit CRC (normal form).
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param forward_polynomial
 * 32-bit polynomial (forward, _bits not reversed_) for which the LUT shall be created.\n
 * Example: X^32+X^26+X^23+X^22+X^16+X^12+X^11+X^10+X^8+X^7+X^5+X^4+X^2+X^1+1 -> 0x04C11DB7
 *
 * \param table
 * The LUT is written into this.
 */
void GenerateCRC32Table_normal(uint32_t const forward_polynomial, uint32_t table[256]) noexcept
{
  for (uint_fast32_t i = 0U; i < 256U; i++)
  {
    uint_fast32_t crc = i << 24U;

    for (uint_fast8_t j = 0U; j < 8U; j++)
    {
      if ((crc & 0x80000000UL) != 0U)
        crc = (crc << 1U) ^ forward_polynomial;
      else
        crc <<= 1U;
    }

    table[i] = crc;
  }
}

/**
 * \ingroup GPCC_CRC
 * \brief Generates a LUT for a 32-bit CRC (reflected form).
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param reverse_polynomial
 * 32-bit polynomial (_bits reversed_) for which the LUT shall be created.\n
 * Example: X^32+X^26+X^23+X^22+X^16+X^12+X^11+X^10+X^8+X^7+X^5+X^4+X^2+X^1+1 -> 0xEDB88320
 *
 * \param table
 * The LUT is written into this.
 */
void GenerateCRC32Table_reflected(uint32_t const reverse_polynomial, uint32_t table[256]) noexcept
{
  for (uint_fast32_t i = 0U; i < 256U; i++)
  {
    uint_fast32_t crc = i;

    for (uint_fast8_t j = 0U; j < 8U; j++)
    {
      if ((crc & 0x1U) != 0U)
        crc = (crc >> 1U) ^ reverse_polynomial;
      else
        crc >>= 1U;
    }

    table[i] = crc;
  }
}

/**
 * \ingroup GPCC_CRC
 * \brief Includes a single byte into a 8 bit CRC _without reversal_ of input data bits.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param crc
 * Reference to the variable containing the checksum.\n
 * At the beginning of the calculation this must be initialized with the proper start value, which depends on the type
 * of CRC. Depending on the type of CRC and the form (normal/reflected), the bits of the final CRC may need to be
 * reversed and/or XOR'd with some value.
 *
 * \param data
 * Data byte that shall be included in the checksum.\n
 * The bits of the data will not be reversed.
 *
 * \param table
 * Table containing the CRC LUT that shall be used.\n
 * Either a predefined table (e.g. @ref crc8_ccitt_table_normal) or a generated one (see @ref GenerateCRC8Table_normal() and
 * @ref GenerateCRC8Table_reflected()) can be used.
 */
void CalcCRC8_noInputReverse(uint8_t & crc, uint8_t const data, uint8_t const table[256]) noexcept
{
  crc = table[crc ^ data];
}

/**
 * \ingroup GPCC_CRC
 * \brief Includes a chunk of bytes into a 8 bit CRC _without reversal_ of input data bits.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param crc
 * Reference to the variable containing the checksum.\n
 * At the beginning of the calculation this must be initialized with the proper start value, which depends on the type
 * of CRC. Depending on the type of CRC and the form (normal/reflected), the bits of the final CRC may need to be
 * reversed and/or XOR'd with some value.
 *
 * \param pData
 * Pointer to the data that shall be included in the checksum.\n
 * The bits of the data will not be reversed.\n
 * The data is included in the checksum byte by byte.
 *
 * \param n
 * Number of bytes. Zero is allowed.
 *
 * \param table
 * Table containing the CRC LUT that shall be used.\n
 * Either a predefined table (e.g. @ref crc8_ccitt_table_normal) or a generated one (see @ref GenerateCRC8Table_normal() and
 * @ref GenerateCRC8Table_reflected()) can be used.
 */
void CalcCRC8_noInputReverse(uint8_t & crc, void const * const pData, size_t n, uint8_t const table[256]) noexcept
{
  uint8_t const * p = static_cast<uint8_t const *>(pData);

  while (n-- != 0)
    crc = table[crc ^ (*p++)];
}

/**
 * \ingroup GPCC_CRC
 * \brief Includes a single byte into a 8 bit CRC _with reversal_ of input data bits.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param crc
 * Reference to the variable containing the checksum.\n
 * At the beginning of the calculation this must be initialized with the proper start value, which depends on the type
 * of CRC. Depending on the type of CRC and the form (normal/reflected), the bits of the final CRC may need to be
 * reversed and/or XOR'd with some value.
 *
 * \param data
 * Data byte that shall be included in the checksum.\n
 * The bits of the data will be reversed.
 *
 * \param table
 * Table containing the CRC LUT that shall be used.\n
 * Either a predefined table (e.g. @ref crc8_ccitt_table_normal) or a generated one (see @ref GenerateCRC8Table_normal() and
 * @ref GenerateCRC8Table_reflected()) can be used.
 */
void CalcCRC8_withInputReverse(uint8_t & crc, uint8_t const data, uint8_t const table[256]) noexcept
{
  crc = table[crc ^ gpcc::Compiler::ReverseBits8(data)];
}

/**
 * \ingroup GPCC_CRC
 * \brief Includes a chunk of bytes into a 8 bit CRC _with reversal_ of input data bits.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param crc
 * Reference to the variable containing the checksum.\n
 * At the beginning of the calculation this must be initialized with the proper start value, which depends on the type
 * of CRC. Depending on the type of CRC and the form (normal/reflected), the bits of the final CRC may need to be
 * reversed and/or XOR'd with some value.
 *
 * \param pData
 * Pointer to the data that shall be included in the checksum.\n
 * The bits of the data will be reversed.\n
 * The data is included in the checksum byte by byte.
 *
 * \param n
 * Number of bytes. Zero is allowed.
 *
 * \param table
 * Table containing the CRC LUT that shall be used.\n
 * Either a predefined table (e.g. @ref crc8_ccitt_table_normal) or a generated one (see @ref GenerateCRC8Table_normal() and
 * @ref GenerateCRC8Table_reflected()) can be used.
 */
void CalcCRC8_withInputReverse(uint8_t & crc, void const * const pData, size_t n, uint8_t const table[256]) noexcept
{
  uint8_t const * p = static_cast<uint8_t const *>(pData);

  while (n-- != 0)
    crc = table[crc ^ gpcc::Compiler::ReverseBits8(*p++)];
}

/**
 * \ingroup GPCC_CRC
 * \brief Includes a single byte into a 16 bit CRC (normal form) _without reversal_ of input data bits.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param crc
 * Reference to the variable containing the checksum.\n
 * At the beginning of the calculation this must be initialized with the proper start value, which depends on the type
 * of CRC. Also depending on the type of CRC, the bits of the final CRC may need to be reversed and/or XOR'd with some
 * value.
 *
 * \param data
 * Data byte that shall be included in the checksum.\n
 * The bits of the data _will not_ be reversed.
 *
 * \param table
 * Table containing the CRC LUT that shall be used.\n
 * Either a predefined table (e.g. @ref crc16_ccitt_table_normal) or a generated one
 * (see @ref GenerateCRC16Table_normal()) can be used.
 */
void CalcCRC16_normal_noInputReverse(uint16_t & crc, uint8_t const data, uint16_t const table[256]) noexcept
{
  crc = (crc << 8U) ^ table[(crc >> 8U) ^ data];
}

/**
 * \ingroup GPCC_CRC
 * \brief Includes a chunk of bytes into a 16 bit CRC (normal form) _without reversal_ of input data bits.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param crc
 * Reference to the variable containing the checksum.\n
 * At the beginning of the calculation this must be initialized with the proper start value, which depends on the type
 * of CRC. Also depending on the type of CRC, the bits of the final CRC may need to be reversed and/or XOR'd with some
 * value.
 *
 * \param pData
 * Pointer to the data that shall be included in the checksum.\n
 * The bits of the data _will not_ be reversed.\n
 * The data is included in the checksum byte by byte.
 *
 * \param n
 * Number of bytes. Zero is allowed.
 *
 * \param table
 * Table containing the CRC LUT that shall be used.\n
 * Either a predefined table (e.g. @ref crc16_ccitt_table_normal) or a generated one
 * (see @ref GenerateCRC16Table_normal()) can be used.
 */
void CalcCRC16_normal_noInputReverse(uint16_t & crc, void const * const pData, size_t n, uint16_t const table[256]) noexcept
{
  uint8_t const * p = static_cast<uint8_t const *>(pData);

  while (n-- != 0)
    CalcCRC16_normal_noInputReverse(crc, *p++, table);
}

/**
 * \ingroup GPCC_CRC
 * \brief Includes a single byte into a 16 bit CRC (normal form) _with reversal_ of input data bits.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param crc
 * Reference to the variable containing the checksum.\n
 * At the beginning of the calculation this must be initialized with the proper start value, which depends on the type
 * of CRC. Also depending on the type of CRC, the bits of the final CRC may need to be reversed and/or XOR'd with some
 * value.
 *
 * \param data
 * Data byte that shall be included in the checksum.\n
 * The bits of the data _will_ be reversed.
 *
 * \param table
 * Table containing the CRC LUT that shall be used.\n
 * Either a predefined table (e.g. @ref crc16_ccitt_table_normal) or a generated one
 * (see @ref GenerateCRC16Table_normal()) can be used.
 */
void CalcCRC16_normal_withInputReverse(uint16_t & crc, uint8_t const data, uint16_t const table[256]) noexcept
{
  crc = (crc << 8U) ^ table[(crc >> 8U) ^ gpcc::Compiler::ReverseBits8(data)];
}

/**
 * \ingroup GPCC_CRC
 * \brief Includes a chunk of bytes into a 16 bit CRC (normal form) _with reversal_ of input data bits.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param crc
 * Reference to the variable containing the checksum.\n
 * At the beginning of the calculation this must be initialized with the proper start value, which depends on the type
 * of CRC. Also depending on the type of CRC, the bits of the final CRC may need to be reversed and/or XOR'd with some
 * value.
 *
 * \param pData
 * Pointer to the data that shall be included in the checksum.\n
 * The bits of the data _will_ be reversed.\n
 * The data is included in the checksum byte by byte.
 *
 * \param n
 * Number of bytes. Zero is allowed.
 *
 * \param table
 * Either a predefined table (e.g. @ref crc16_ccitt_table_normal) or a generated one
 * (see @ref GenerateCRC16Table_normal()) can be used.
 */
void CalcCRC16_normal_withInputReverse(uint16_t & crc, void const * const pData, size_t n, uint16_t const table[256]) noexcept
{
  uint8_t const * p = static_cast<uint8_t const *>(pData);

  while (n-- != 0)
    CalcCRC16_normal_withInputReverse(crc, *p++, table);
}

/**
 * \ingroup GPCC_CRC
 * \brief Includes a single byte into a 16 bit CRC (reflected form) _without reversal_ of input data bits.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param crc
 * Reference to the variable containing the checksum.\n
 * At the beginning of the calculation this must be initialized with the proper start value, which depends on the type
 * of CRC. Also depending on the type of CRC, the bits of the final CRC may need to be reversed and/or XOR'd with some
 * value.
 *
 * \param data
 * Data byte that shall be included in the checksum.\n
 * The bits of the data _will not_ be reversed.
 *
 * \param table
 * Table containing the CRC LUT that shall be used.\n
 * @ref GenerateCRC16Table_reflected() can be used to create a table.
 */
void CalcCRC16_reflected_noInputReverse(uint16_t & crc, uint8_t const data, uint16_t const table[256]) noexcept
{
  crc = (crc >> 8U) ^ table[(crc & 0xFFU) ^ data];
}

/**
 * \ingroup GPCC_CRC
 * \brief Includes a chunk of bytes into a 16 bit CRC (reflected form) _without reversal_ of input data bits.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param crc
 * Reference to the variable containing the checksum.\n
 * At the beginning of the calculation this must be initialized with the proper start value, which depends on the type
 * of CRC. Also depending on the type of CRC, the bits of the final CRC may need to be reversed and/or XOR'd with some
 * value.
 *
 * \param pData
 * Pointer to the data that shall be included in the checksum.\n
 * The bits of the data _will not_ be reversed.\n
 * The data is included in the checksum byte by byte.
 *
 * \param n
 * Number of bytes. Zero is allowed.
 *
 * \param table
 * Table containing the CRC LUT that shall be used.\n
 * @ref GenerateCRC16Table_reflected() can be used to create a table.
 */
void CalcCRC16_reflected_noInputReverse(uint16_t & crc, void const * const pData, size_t n, uint16_t const table[256]) noexcept
{
  uint8_t const * p = static_cast<uint8_t const *>(pData);

  while (n-- != 0)
    CalcCRC16_reflected_noInputReverse(crc, *p++, table);
}

/**
 * \ingroup GPCC_CRC
 * \brief Includes a single byte into a 16 bit CRC (reflected form) _with reversal_ of input data bits.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param crc
 * Reference to the variable containing the checksum.\n
 * At the beginning of the calculation this must be initialized with the proper start value, which depends on the type
 * of CRC. Also depending on the type of CRC, the bits of the final CRC may need to be reversed and/or XOR'd with some
 * value.
 *
 * \param data
 * Data byte that shall be included in the checksum.\n
 * The bits of the data _will_ be reversed.
 *
 * \param table
 * Table containing the CRC LUT that shall be used.\n
 * @ref GenerateCRC16Table_reflected() can be used to create a table.
 */
void CalcCRC16_reflected_withInputReverse(uint16_t & crc, uint8_t const data, uint16_t const table[256]) noexcept
{
  crc = (crc >> 8U) ^ table[(crc & 0xFFU) ^ gpcc::Compiler::ReverseBits8(data)];
}

/**
 * \ingroup GPCC_CRC
 * \brief Includes a chunk of bytes into a 16 bit CRC (reflected form) _with reversal_ of input data bits.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param crc
 * Reference to the variable containing the checksum.\n
 * At the beginning of the calculation this must be initialized with the proper start value, which depends on the type
 * of CRC. Also depending on the type of CRC, the bits of the final CRC may need to be reversed and/or XOR'd with some
 * value.
 *
 * \param pData
 * Pointer to the data that shall be included in the checksum.\n
 * The bits of the data _will_ be reversed.\n
 * The data is included in the checksum byte by byte.
 *
 * \param n
 * Number of bytes. Zero is allowed.
 *
 * \param table
 * Table containing the CRC LUT that shall be used.\n
 * @ref GenerateCRC16Table_reflected() can be used to create a table.
 */
void CalcCRC16_reflected_withInputReverse(uint16_t & crc, void const * const pData, size_t n, uint16_t const table[256]) noexcept
{
  uint8_t const * p = static_cast<uint8_t const *>(pData);

  while (n-- != 0)
    CalcCRC16_reflected_withInputReverse(crc, *p++, table);
}

/**
 * \ingroup GPCC_CRC
 * \brief Includes a single byte into a 32 bit CRC (normal form) _without reversal_ of input data bits.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param crc
 * Reference to the variable containing the checksum.\n
 * At the beginning of the calculation this must be initialized with the proper start value, which depends on the type
 * of CRC. Also depending on the type of CRC, the bits of the final CRC may need to be reversed and/or XOR'd with some
 * value.
 *
 * \param data
 * Data byte that shall be included in the checksum.\n
 * The bits of the data _will not_ be reversed.
 *
 * \param table
 * Table containing the CRC LUT that shall be used.\n
 * Either a predefined table (e.g. @ref crc32ab_table_normal) or a generated one (see @ref GenerateCRC32Table_normal())
 * can be used.
 */
void CalcCRC32_normal_noInputReverse(uint32_t & crc, uint8_t const data, uint32_t const table[256]) noexcept
{
  crc = (crc << 8U) ^ table[(crc >> 24U) ^ data];
}

/**
 * \ingroup GPCC_CRC
 * \brief Includes a chunk of bytes into a 32 bit CRC (normal form) _without reversal_ of input data bits.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param crc
 * Reference to the variable containing the checksum.\n
 * At the beginning of the calculation this must be initialized with the proper start value, which depends on the type
 * of CRC. Also depending on the type of CRC, the bits of the final CRC may need to be reversed and/or XOR'd with some
 * value.
 *
 * \param pData
 * Pointer to the data that shall be included in the checksum.\n
 * The bits of the data _will not_ be reversed.\n
 * The data is included in the checksum byte by byte.
 *
 * \param n
 * Number of bytes. Zero is allowed.
 *
 * \param table
 * Table containing the CRC LUT that shall be used.\n
 * Either a predefined table (e.g. @ref crc32ab_table_normal) or a generated one (see @ref GenerateCRC32Table_normal())
 * can be used.
 */
void CalcCRC32_normal_noInputReverse(uint32_t & crc, void const * const pData, size_t n, uint32_t const table[256]) noexcept
{
  uint8_t const * p = static_cast<uint8_t const *>(pData);

  while (n-- != 0)
    CalcCRC32_normal_noInputReverse(crc, *p++, table);
}

/**
 * \ingroup GPCC_CRC
 * \brief Includes a single byte into a 32 bit CRC (normal form) _with reversal_ of input data bits.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param crc
 * Reference to the variable containing the checksum.\n
 * At the beginning of the calculation this must be initialized with the proper start value, which depends on the type
 * of CRC. Also depending on the type of CRC, the bits of the final CRC may need to be reversed and/or XOR'd with some
 * value.
 *
 * \param data
 * Data byte that shall be included in the checksum.\n
 * The bits of the data _will_ be reversed.
 *
 * \param table
 * Table containing the CRC LUT that shall be used.\n
 * Either a predefined table (e.g. @ref crc32ab_table_normal) or a generated one (see @ref GenerateCRC32Table_normal())
 * can be used.
 */
void CalcCRC32_normal_withInputReverse(uint32_t & crc, uint8_t const data, uint32_t const table[256]) noexcept
{
  crc = (crc << 8U) ^ table[(crc >> 24U) ^ gpcc::Compiler::ReverseBits8(data)];
}

/**
 * \ingroup GPCC_CRC
 * \brief Includes a chunk of bytes into a 32 bit CRC (normal form) _with reversal_ of input data bits.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param crc
 * Reference to the variable containing the checksum.\n
 * At the beginning of the calculation this must be initialized with the proper start value, which depends on the type
 * of CRC. Also depending on the type of CRC, the bits of the final CRC may need to be reversed and/or XOR'd with some
 * value.
 *
 * \param pData
 * Pointer to the data that shall be included in the checksum.\n
 * The bits of the data _will_ be reversed.\n
 * The data is included in the checksum byte by byte.
 *
 * \param n
 * Number of bytes. Zero is allowed.
 *
 * \param table
 * Table containing the CRC LUT that shall be used.\n
 * Either a predefined table (e.g. @ref crc32ab_table_normal) or a generated one (see @ref GenerateCRC32Table_normal())
 * can be used.
 */
void CalcCRC32_normal_withInputReverse(uint32_t & crc, void const * const pData, size_t n, uint32_t const table[256]) noexcept
{
  uint8_t const * p = static_cast<uint8_t const *>(pData);

  while (n-- != 0)
    CalcCRC32_normal_withInputReverse(crc, *p++, table);
}

/**
 * \ingroup GPCC_CRC
 * \brief Includes a single byte into a 32 bit CRC (reflected form) _without reversal_ of input data bits.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param crc
 * Reference to the variable containing the checksum.\n
 * At the beginning of the calculation this must be initialized with the proper start value, which depends on the type
 * of CRC. Also depending on the type of CRC, the bits of the final CRC may need to be reversed and/or XOR'd with some
 * value.
 *
 * \param data
 * Data byte that shall be included in the checksum.\n
 * The bits of the data _will not_ be reversed.
 *
 * \param table
 * Table containing the CRC LUT that shall be used.\n
 * Either a predefined table (e.g. @ref crc32ab_table_reflected) or a generated one
 * (see @ref GenerateCRC32Table_reflected()) can be used.
 */
void CalcCRC32_reflected_noInputReverse(uint32_t & crc, uint8_t const data, uint32_t const table[256]) noexcept
{
  crc = (crc >> 8U) ^ table[(crc & 0xFFU) ^ data];
}

/**
 * \ingroup GPCC_CRC
 * \brief Includes a chunk of bytes into a 32 bit CRC (reflected form) _without reversal_ of input data bits.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param crc
 * Reference to the variable containing the checksum.\n
 * At the beginning of the calculation this must be initialized with the proper start value, which depends on the type
 * of CRC. Also depending on the type of CRC, the bits of the final CRC may need to be reversed and/or XOR'd with some
 * value.
 *
 * \param pData
 * Pointer to the data that shall be included in the checksum.\n
 * The bits of the data _will not_ be reversed.\n
 * The data is included in the checksum byte by byte.
 *
 * \param n
 * Number of bytes. Zero is allowed.
 *
 * \param table
 * Table containing the CRC LUT that shall be used.\n
 * Either a predefined table (e.g. @ref crc32ab_table_reflected) or a generated one
 * (see @ref GenerateCRC32Table_reflected()) can be used.
 */
void CalcCRC32_reflected_noInputReverse(uint32_t & crc, void const * const pData, size_t n, uint32_t const table[256]) noexcept
{
  uint8_t const * p = static_cast<uint8_t const *>(pData);

  while (n-- != 0)
    CalcCRC32_reflected_noInputReverse(crc, *p++, table);
}

/**
 * \ingroup GPCC_CRC
 * \brief Includes a single byte into a 32 bit CRC (reflected form) _with reversal_ of input data bits.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param crc
 * Reference to the variable containing the checksum.\n
 * At the beginning of the calculation this must be initialized with the proper start value, which depends on the type
 * of CRC. Also depending on the type of CRC, the bits of the final CRC may need to be reversed and/or XOR'd with some
 * value.
 *
 * \param data
 * Data byte that shall be included in the checksum.\n
 * The bits of the data _will_ be reversed.
 *
 * \param table
 * Table containing the CRC LUT that shall be used.\n
 * Either a predefined table (e.g. @ref crc32ab_table_reflected) or a generated one
 * (see @ref GenerateCRC32Table_reflected()) can be used.
 */
void CalcCRC32_reflected_withInputReverse(uint32_t & crc, uint8_t const data, uint32_t const table[256]) noexcept
{
  crc = (crc >> 8U) ^ table[(crc & 0xFFU) ^ gpcc::Compiler::ReverseBits8(data)];
}

/**
 * \ingroup GPCC_CRC
 * \brief Includes a chunk of bytes into a 32 bit CRC (reflected form) _with reversal_ of input data bits.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param crc
 * Reference to the variable containing the checksum.\n
 * At the beginning of the calculation this must be initialized with the proper start value, which depends on the type
 * of CRC. Also depending on the type of CRC, the bits of the final CRC may need to be reversed and/or XOR'd with some
 * value.
 *
 * \param pData
 * Pointer to the data that shall be included in the checksum.\n
 * The bits of the data _will_ be reversed.\n
 * The data is included in the checksum byte by byte.
 *
 * \param n
 * Number of bytes. Zero is allowed.
 *
 * \param table
 * Table containing the CRC LUT that shall be used.\n
 * Either a predefined table (e.g. @ref crc32ab_table_reflected) or a generated one
 * (see @ref GenerateCRC32Table_reflected()) can be used.
 */
void CalcCRC32_reflected_withInputReverse(uint32_t & crc, void const * const pData, size_t n, uint32_t const table[256]) noexcept
{
  uint8_t const * p = static_cast<uint8_t const *>(pData);

  while (n-- != 0)
    CalcCRC32_reflected_withInputReverse(crc, *p++, table);
}

} // namespace crc
} // namespace gpcc
