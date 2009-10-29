/* -*- Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 University of Washington
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * Author:  Craig Dowell (craigdo@ee.washington.edu)
 */

#ifndef PCAP_FILE_H
#define PCAP_FILE_H

#include <string>
#include <stdint.h>

namespace ns3 {

/*
 * A class representing a pcap file.  This allows easy creation, writing and 
 * reading of files composed of stored packets; which may be viewed using
 * standard tools.
 */

class PcapFile
{
public:
  static const int32_t  ZONE_DEFAULT    = 0;           /**< Time zone offset for current location */
  static const uint32_t SNAPLEN_DEFAULT = 65535;       /**< Default value for maximum octets to save per packet */

public:
  PcapFile ();
  ~PcapFile ();

  /**
   * Create a new pcap file or open an existing pcap file.  Semantics are
   * similar to the C standard library function \c fopen, but differ in that
   * positions in the file are based on packets not characters.  For example
   * if the file is opened for reading, the file position indicator (seek
   * position) points to the beginning of the first packet in the file, not
   * zero (which would point to the start of the pcap header).
   *
   * Possible modes are:
   *
   * \verbatim
   * "r":   Open a file for reading.  The file must exist.  The pcap header
   *        is assumed to exist in the file and will be read and checked.
   *        The file seek position indicator is set to point to the first 
   *        packet on exit.
   *
   * "w":   Create an empty file for writing. If a file with the same name 
   *        already exists its content is erased and the file is treated as a 
   *        new empty pcap file.  The file is assumed not to have a pcap 
   *        header and the caller is responsible for calling Init before saving
   *        any packet data.  The file seek position indicator is set to point 
   *        to the beginning of the file on exit since there will be no pcap
   *        header.
   *
   * "a":   Append to an existing file. This mode allows for adding packet data
   *        to the end of an existing pcap file.  The file must exist and have a
   *        valid pcap header written (N.B. this is different from standard fopen
   *        semantics).  The file seek position indicator is set to point 
   *        to the end of the file on exit.
   *
   * "r+":  Open a file for update -- both reading and writing. The file must 
   *        exist.  The pcap header is assumed to have been written to the 
   *        file and will be read and checked.  The file seek position indicator
   *        is set to point to the first packet on exit.
   *
   * "w+":  Create an empty file for both reading and writing.  If a file with
   *        the same name already exists, its content is erased and the file is 
   *        treated as a new empty pcap file.  Since this new file will not have
   *        a pcap header, the caller is responsible for calling Init before 
   *        saving any packet data.  On exit, the file seek position indicator is
   *        set to point to the beginning of the file.
   *
   * "a+"   Open a file for reading and appending.  The file must exist and have a
   *        valid pcap header written (N.B. this is different from standard fopen
   *        semantics).  The file seek position indicator is set to point 
   *        to the end of the file on exit.  Existing content is preserved.
   * \endverbatim
   *
   * Since a pcap file is always a binary file, the file type is automatically 
   * selected as a binary file.  For example, providing a mode string "a+" 
   * results in the underlying OS file being opened in "a+b" mode.
   *
   * \param filename String containing the name of the file.
   *
   * \param mode String containing the access mode for the file.
   *
   * \returns Error indication that should be interpreted as, "did an error 
   * happen"?  That is, the method returns false if the open succeeds, true 
   * otherwise.  The errno variable will be set by the OS to to provide a 
   * more descriptive failure indication.
   */
  bool Open (std::string const &filename, std::string const &mode);

  void Close (void);

  /**
   * Initialize the pcap file associated with this object.  This file must have
   * been previously opened with write permissions.
   *
   * \param dataLinkType A data link type as defined in the pcap library.  If
   * you want to make resulting pcap files visible in existing tools, the 
   * data link type must match existing definitions, such as PCAP_ETHERNET,
   * PCAP_PPP, PCAP_80211, etc.  If you are storing different kinds of packet
   * data, such as naked TCP headers, you are at liberty to locally define your
   * own data link types.  According to the pcap-linktype man page, "well-known"
   * pcap linktypes range from 0 to 177.  If you use a large random number for
   * your type, chances are small for a collision.
   *
   * \param snapLen An optional maximum size for packets written to the file.
   * Defaults to 65535.  If packets exceed this length they are truncated.
   *
   * \param timeZoneCorrection An integer describing the offset of your local
   * time zone from UTC/GMT.  For example, Pacific Standard Time in the US is
   * GMT-8, so one would enter -8 for that correction.  Defaults to 0 (UTC).
   *
   * \return false if the open succeeds, true otherwise.
   *
   * \warning Calling this method on an existing file will result in the loss
   * any existing data.
   */
  bool Init (uint32_t dataLinkType, 
             uint32_t snapLen = SNAPLEN_DEFAULT, 
             int32_t timeZoneCorrection = ZONE_DEFAULT,
             bool swapMode = false);

  /**
   * \brief Write next packet to file
   * 
   * \param tsSec       Packet timestamp, seconds 
   * \param tsUsec      Packet timestamp, microseconds
   * \param data        Data buffer
   * \param totalLen    Total packet length
   * 
   * \return true on error, false otherwise
   */
  bool Write (uint32_t tsSec, uint32_t tsUsec, uint8_t const * const data, uint32_t totalLen);

  /**
   * \brief Read next packet from file
   * 
   * \param data        [out] Data buffer
   * \param maxBytes    Allocated data buffer size
   * \param tsSec       [out] Packet timestamp, seconds
   * \param tsUsec      [out] Packet timestamp, microseconds
   * \param inclLen     [out] Included length
   * \param origLen     [out] Original length
   * \param readLen     [out] Number of bytes read
   * 
   * \return true if read failed, false otherwise
   */
  bool Read (uint8_t * const data, 
             uint32_t maxBytes,
             uint32_t &tsSec, 
             uint32_t &tsUsec, 
             uint32_t &inclLen, 
             uint32_t &origLen, 
             uint32_t &readLen);

  bool GetSwapMode (void);

  uint32_t GetMagic (void);
  uint16_t GetVersionMajor (void);
  uint16_t GetVersionMinor (void);
  int32_t GetTimeZoneOffset (void);
  uint32_t GetSigFigs (void);
  uint32_t GetSnapLen (void);
  uint32_t GetDataLinkType (void);
  
  /**
   * \brief Compare two PCAP files packet-by-packet
   * 
   * \return true if files are different, false otherwise
   * 
   * \param  f1         First PCAP file name
   * \param  f2         Second PCAP file name
   * \param  sec        [out] Time stamp of first different packet, seconds. Undefined if files doesn't differ.
   * \param  uses       [out] Time stamp of first different packet, microseconds. Undefined if files doesn't differ.
   * \param  snapLen    Snap length (if used)
   */
  static bool Diff (std::string const & f1, std::string const & f2, 
                    uint32_t & sec, uint32_t & usec, 
                    uint32_t snapLen = SNAPLEN_DEFAULT);

private:
  typedef struct {
    uint32_t m_magicNumber;   /**< Magic number identifying this as a pcap file */
    uint16_t m_versionMajor;  /**< Major version identifying the version of pcap used in this file */
    uint16_t m_versionMinor;  /**< Minor version identifying the version of pcap used in this file */
    int32_t  m_zone;          /**< Time zone correction to be applied to timestamps of packets */
    uint32_t m_sigFigs;       /**< Unused by pretty much everybody */
    uint32_t m_snapLen;       /**< Maximum length of packet data stored in records */
    uint32_t m_type;          /**< Data link type of packet data */
  } PcapFileHeader;

  typedef struct {
    uint32_t m_tsSec;         /**< seconds part of timestamp */
    uint32_t m_tsUsec;        /**< microseconds part of timestamp (nsecs for PCAP_NSEC_MAGIC) */
    uint32_t m_inclLen;       /**< number of octets of packet saved in file */
    uint32_t m_origLen;       /**< actual length of original packet */
  } PcapRecordHeader;

  uint8_t Swap (uint8_t val);
  uint16_t Swap (uint16_t val);
  uint32_t Swap (uint32_t val);
  void Swap (PcapFileHeader *from, PcapFileHeader *to);
  void Swap (PcapRecordHeader *from, PcapRecordHeader *to);

  bool WriteFileHeader (void);
  bool ReadAndVerifyFileHeader (void);

  std::string    m_filename;
  FILE          *m_filePtr;
  PcapFileHeader m_fileHeader;
  bool m_haveFileHeader;
  bool m_swapMode;
};

}//namespace ns3

#endif /* PCAP_FILE_H */