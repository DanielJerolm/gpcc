/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef OBJECTINFORESPONSE_HPP_202102132033
#define OBJECTINFORESPONSE_HPP_202102132033

#include <gpcc/cood/remote_access/requests_and_responses/ResponseBase.hpp>
#include <gpcc/cood/data_types.hpp>
#include <gpcc/cood/Object.hpp>
#include <gpcc/cood/sdo_abort_codes.hpp>

namespace gpcc {
namespace cood {

/**
 * \ingroup GPCC_COOD_REMOTEACCESS_REQRESP
 * \brief Key for public constructor of class @ref ObjectInfoResponsePassKey (passkey-pattern).
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class ObjectInfoResponsePassKey final
{
  private:
    friend class ResponseBase;

    ObjectInfoResponsePassKey(void) = default;
};

/**
 * \ingroup GPCC_COOD_REMOTEACCESS_REQRESP
 * \brief Object dictionary remote access request response:\n
 *        Object meta data query response.
 *
 * This is the response transmitted by the remote access server when a @ref ObjectInfoRequest is processed.
 *
 * # Usage (for clients)
 * ## Extract result of query request
 * 1. Use @ref GetResult() to check for any error.
 * 2. Use @ref IsComplete() to check if defragmentation is requried. See chapter "Fragmentation" below for details.
 * 3. If required: Perform fragmented transfer.
 * 4. Use @ref GetFirstQueriedSubindex() and @ref GetLastQueriedSubindex() to determine the range of subindices
 *    whose meta data is contained in the response.
 * 5. Use the Getter-methods offered by this class to retrieve the meta data of the object and its subindices.
 *
 * ## Fragmentation
 * This type of response supports fragmentation:\n
 * The client shall issue a @ref ObjectInfoRequest which queries the desired range of subindices. When the response is
 * received, the client shall use @ref GetResult() to query if the response is OK. If it is OK, then the client shall
 * use @ref IsComplete() to figure out if the response contains the meta data of all queried subindices, or not. If
 * the response is incomplete, then @ref IsComplete() will also tell at which subindex the query shall be continued.
 * The client shall issue a second @ref ObjectInfoRequest which starts querying the meta data at that subindex and which
 * extends to the desired last subindex where the query shall stop.
 *
 * After reception of the response of the second @ref ObjectInfoResponse, @ref GetResult() shall be used to examine
 * the status of the second response. If the status is OK, then @ref AddFragment() shall be used to merge the
 * __second__ response into the __first__ response. After the merge, the client shall use @ref IsComplete() on the
 * __first__ response to determine if the __first__ response is complete now and -if it is not- at which subindex the
 * query shall continue.
 *
 * The client shall repeat issuing requests and merging responses into the __very first__ response until
 * @ref IsComplete() invoked on the __very first__ response indicates that the meta data of all subindices in the
 * desired range have been queried. The defragmented response can then be read from the __very first__ response.
 *
 * # Usage (for server)
 * ## Happy path
 * Use constructor `ObjectInfoResponse(7 args)` to query the meta data and create a response object. The constructor
 * will query as many subindices as possible until the desired number of subindices has been queried or the maximum
 * response size is reached.
 *
 * ## Error scenario
 * Use constructor @ref ObjectInfoResponse(SDOAbortCode const _result) to create a response containing an appropriate
 * error code.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class ObjectInfoResponse final : public ResponseBase
{
  public:
    ObjectInfoResponse(void) = delete;

    explicit ObjectInfoResponse(SDOAbortCode const _result);
    explicit ObjectInfoResponse(Object const & obj,
                                uint8_t const _firstSubindex,
                                uint8_t lastSubindex,
                                bool const _inclusiveNames,
                                bool const _inclusiveAppSpecificMetaData,
                                size_t const maxResponseSize,
                                size_t const returnStackSize);

    ObjectInfoResponse(gpcc::stream::IStreamReader & sr, uint8_t const versionOnHand, ObjectInfoResponsePassKey);

    ObjectInfoResponse(ObjectInfoResponse const &) = default;
    ObjectInfoResponse(ObjectInfoResponse &&) = default;

    ~ObjectInfoResponse(void) = default;

    ObjectInfoResponse& operator=(ObjectInfoResponse const &) = delete;
    ObjectInfoResponse& operator=(ObjectInfoResponse &&) = delete;


    // <-- ResponseBase
    size_t GetBinarySize(void) const override;
    void ToBinary(gpcc::stream::IStreamWriter & sw) const override;

    std::string ToString(void) const override;
    // --> ResponseBase

    void AddFragment(ObjectInfoResponse && fragment);

    SDOAbortCode GetResult(void) const noexcept;
    bool IsInclusiveNames(void) const noexcept;
    bool IsInclusiveAppSpecificMetaData(void) const noexcept;

    uint8_t GetFirstQueriedSubindex(void) const;
    uint8_t GetLastQueriedSubindex(void) const;
    bool IsComplete(uint8_t * const pNextSubindex) const;

    Object::ObjectCode GetObjectCode(void) const;
    DataType GetObjectDataType(void) const;
    std::string const & GetObjectName(void) const;
    uint16_t GetMaxNbOfSubindices(void) const;

    bool IsSubIndexEmpty(uint8_t const subIdx) const;
    DataType GetSubIdxDataType(uint8_t const subIdx) const;
    Object::attr_t GetSubIdxAttributes(uint8_t const subIdx) const;
    size_t GetSubIdxMaxSize(uint8_t const subIdx) const;
    std::string GetSubIdxName(uint8_t const subIdx) const;

    size_t GetAppSpecificMetaDataSize(uint8_t const subIdx) const;
    std::vector<uint8_t> GetAppSpecificMetaData(uint8_t const subIdx) const;

  private:
    /// Container for meta data queried from a single subindex.
    struct SubindexDescr final
    {
      public:
        bool empty;                           ///<Indicates if the subindex is empty.
        bool inclName;                        ///<Indicates if the name of the subindex is included.
        bool inclASM;                         ///<Indicates if application specific meta data of the subindex is included.
        bool maxSizeU8;                       ///<Indicates if @ref maxSize is encoded as U8 (true) or U32 (false) in binary.
        bool appSpecMetaDataSizeU8;           ///<Indicates if the size of @ref appSpecMetaData is encoded as U8 (true) or U32 (false) in binary.
        DataType dataType;                    ///<CANopen data type of the subindex.
        Object::attr_t attributes;            ///<Attributes of the subindex.
        size_t maxSize;                       ///<Maximum size of the subindex in bit.
        std::string name;                     ///<Name of the subindex.
        std::vector<uint8_t> appSpecMetaData; ///<Application specific meta data of the subindex.

        SubindexDescr(void) = delete;
        SubindexDescr(Object const & obj, uint8_t const subIndex, bool const inclusiveName, bool const inclusiveASM);
        SubindexDescr(gpcc::stream::IStreamReader & sr);
        SubindexDescr(SubindexDescr const &) = default;
        SubindexDescr(SubindexDescr &&) noexcept = default;
        ~SubindexDescr(void) = default;

        SubindexDescr& operator=(SubindexDescr const &) = default;
        SubindexDescr& operator=(SubindexDescr &&) = default;

        size_t GetBinarySize(void) const;
        void ToBinary(gpcc::stream::IStreamWriter & sw) const;
    };


    /// Result of the query operation.
    SDOAbortCode result;

    /// Indicates if object's and subindices' names are included in queried meta data or not.
    /** This is only valid, if @ref result is @ref SDOAbortCode::OK. */
    bool inclusiveNames;

    /// Indicates if application specific meta data of the subindices shall be included in queried meta data or not.
    /** This is only valid, if @ref result is @ref SDOAbortCode::OK. */
    bool inclusiveAppSpecificMetaData;


    /// Object code of the object.
    /** This is only valid, if @ref result is @ref SDOAbortCode::OK. */
    Object::ObjectCode objectCode;

    /// Data type of the object.
    /** This is only valid, if @ref result is @ref SDOAbortCode::OK. */
    DataType objType;

    /// Name of the object.
    /** This is only valid, if @ref result is @ref SDOAbortCode::OK and @ref inclusiveNames is true. */
    std::string objName;

    /// Maximum number of subindices (incl. SI 0).
    /** This is only valid, if @ref result is @ref SDOAbortCode::OK. */
    uint16_t maxNbOfSubindices;

    /// Number of the first subindex described in @ref subindexDescr.
    /** This is only valid, if @ref result is @ref SDOAbortCode::OK. \n
        If @ref inclusiveAppSpecificMetaData is false, then this is 0 or 1 for an ARRAY object. */
    uint8_t firstSubindex;

    /// Subindex descriptions.
    /** This is only valid, if @ref result is @ref SDOAbortCode::OK. \n
        For a VARIABLE object, this contains exactly one item.\n
        For an ARRAY object, this contains 1..2 items, if @ref inclusiveAppSpecificMetaData is false.\n
        For an ARRAy object, this contains 1..256 items, if @ref inclusiveAppSpecificMetaData is true.\n
        For a RECORD object, this contains 1..256 items.\n
        If the @ref ObjectInfoResponse was the source of a move-operation, then this is empty. */
    std::vector<SubindexDescr> subindexDescr;


    void ValidateObjNotEmpty(void) const;
    size_t CalcRemainingPayload(size_t const maxResponseSize, size_t const returnStackSize) const;
    uint8_t MapSubindexToSubIndexDescr(uint8_t const subindex) const;
};

} // namespace cood
} // namespace gpcc

#endif // OBJECTINFORESPONSE_HPP_202102132033
