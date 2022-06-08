/**                                        -*- mode:C++ -*-
 * SourceStream.h -  Basic SourceStream byte handling for ULAM
 *
 * Copyright (C) 2014-2017 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2017 Ackleyshack LLC.
 *
 * This file is part of the ULAM programming language compilation system.
 *
 * The ULAM programming language compilation system is free software:
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * The ULAM programming language compilation system is distributed in
 * the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the ULAM programming language compilation system
 * software.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @license GPL-3.0+ <http://spdx.org/licenses/GPL-3.0+>
 */

/**
  \file SourceStream.h -  Basic SourceStream byte handling for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2017   All rights reserved.
  \gpl
*/


#ifndef SOURCESTREAM_H
#define SOURCESTREAM_H

#include <string>
#include <map>
#include <stack>
#include <vector>
#include "File.h"
#include "FileManager.h"
#include "Locator.h"
#include "CompilerState.h"

namespace MFM{

  class SourceStream
  {

  public:

    SourceStream(FileManager * fm, CompilerState & state);
    ~SourceStream();

    /** Mark last read byte to be "read" again */
    void unread();

    /** Return the next byte of the current file in the SourceStream, as a
	non-negative int.  If EOF is encountered on the current file, it is
	automatically closed and 'popped', and the next byte of the most
	recently suspended file is returned instead, if any remain.  If after
	possibly multiple consecutive 'pops', no further 'push'ed files
	remain, read() returns -1 instead.  If some other I/O problem occurs,
	read() returns a negative number less than -1 according to some
	scheme you devise, without being 'popped'.
    */
    s32 read();


    /** If the given filename has been push'ed before, since this
	SourceStream was constructed, return true.
    */
    bool isPushed(std::string filename);
    bool isPushed(u32 findex);

    /** If onlyOnce is true, cause the SourceStream, first, to check
	if the given filename has been push'ed before, since this
	SourceStream was constructed; If so, the push does nothing and
	returns 0.  Otherwise, SourceStream attempts to open the
	filename for reading.  If that fails, the push does nothing
	and returns nonzero index to error message.  Otherwise, the
	open succeeds, and the SourceStream then suspends reading
	whatever it is currently reading, and begins reading
	'filename' instead (meaning that the next read() call will
	return the first byte of filename, if any, etc).
    */
    u32 push(std::string filename, bool onlyOnce = true);

    /** return 0 when file exists; o.w. non-zero for index of error msg */
    u32 exists(std::string filename);

    /** Get the path (that had previously been push()ed) that was the
	source of the byte returned by the most recent call to read().  If
	read() has never been called on this SourceStream, return NULL.
	If the most recent call to read() returned EOF, return the first
	file that was push()ed into this SourceStream.  If the most recent
	call to read() returned an error, return the path of the File that
	had the error.
    */
    const std::string getPathFromLocator(Locator & loc) const;


    /** Get the Locator by value (that had previously been push()ed)
	that was the source of the byte returned by the most recent
	call to read().  If read() has never been called on this
	SourceStream, return INVALID LOCATOR (id == 0).  If the most
	recent call to read() returned EOF, ie everything popped,
	return the first file that was push()ed into this
	SourceStream.  If the most recent call to read() returned an
	error, return the Locator of the File that had the error.
    */
    Locator getLocator() const;

    /** Get the line number of the byte returned by the most recent call
	to read().  The first line of a file is numbered 1.  The line
	number increments by one during each call to read() that returns
	'\n'.  If read() has never been called on this SourceStream,
	return 0.  Note that when another path is push()ed into a
	SourceStream (for the first time for that path), GetLineNumber()
	will continue to provide the line number of the most recent call
	to read() (in the previous file) until the first call to read()
	following the push().  In addition, if an EOF during read()
	processing causes the SourceStream to 'pop' back to a previous
	path to complete the read(), then a GetLineNumber() call after
	that read() will return the line number associated with the file
	that provided the byte returned from the read().
    */
    u16 getLineNumber() const;



    /** Get the byte number witin the current line of the byte
	returned by the most recent call to read().  If read() has
	never been called on this SourceStream, GetByteNumberInLine
	returns 0.  If the most recent read() call returned '\n',
	GetByteNumberInLine returns 0. Otherwise is returns the number
	of non-'\n' bytes that have been returned by read() since the
	last '\n' was returned.  In effect, the first byte of a line
	is numbered 1, and increases sequentially by one during each
	call to read(), except when read() returns '\n', in which case
	the byte number is reset to 0.  Note that this is a _byte_
	number, not a _column_ number.  In particular, any occurrences
	of the '\t' tab character simply count as one byte, regardless
	of where they may occur on a line.
    */
    u16 getByteNumberInLine() const;


    /** returns Ulam version of the most recently pushed filename; where 0 is unknown */
    u32 getFileUlamVersion() const;

    /** sets Ulam version of the most recently pushed filename */
    void setFileUlamVersion(u32 ver);

    /** retrieves original text by file id and line */
    bool getLineOfTextAsString(Locator loc, std::string & str);

  private:

    struct filerec
    {
      Locator m_loc;
      File * m_fp;
      Locator m_unreadLoc;
      s32 m_unreadByte;
      bool m_haveUnreadByte;
      u32 m_version;
      std::string m_lineText; //e.g. ulam original source as documentation

      filerec() : m_fp(NULL), m_haveUnreadByte(false), m_version(0){}

      ~filerec()
      {
	//closeFile(); //done by File class.
      }

      //void init(u16 idarg, File * fparg, std::string patharg)
      void init(u16 idarg, File * fparg, u32 pathindexarg, u32 fullpathindex)
      {
	//m_loc.setFileId(idarg);  no longer the FileID, use m_registeredFileNames
	// to go from path index to file id.
	m_loc.setPathIndex(pathindexarg);
	m_loc.setFullPathIndex(fullpathindex);
	m_fp = fparg;
	m_unreadLoc = m_loc;
      }

      void closeFile()
      {
	if(m_fp != NULL)
	  {
	    m_fp->close();
	    delete m_fp;
	    m_fp = NULL;
	  }
      }

      void updateUnread(bool haveit, s32 byte, Locator loc)
      {
	m_haveUnreadByte = haveit;
	m_unreadByte = byte;
	m_unreadLoc = loc;
      }

      void updateUnreadLoc(Locator loc)
      {
	m_unreadLoc = loc;
      }

      void setUnreadFlag(bool flag)
      {
	m_haveUnreadByte = flag;
      }

      bool getUnreadFlag() const
      {
	return m_haveUnreadByte;
      }

      s32 getUnreadByte() const
      {
	return m_unreadByte;
      }

      Locator getUnreadLoc() const
      {
	return m_unreadLoc;
      }

      void setFileVersion(u32 ver)
      {
	m_version = ver;
      }

      u32 getFileVersion() const
      {
	return m_version;
      }

      std::string getLineOfText() const
      {
	return m_lineText;
      }

      void appendToLineOfText(char c)
      {
	m_lineText.append(1,c);
      }

      void clearLineOfText()
      {
	m_lineText.clear();
      }

    };  //filerec

    //std::map<std::string, u16> m_registeredFilenames; //maps filenames to id (1..n)
    std::map<u32, u16> m_registeredFilenames; //maps stringpool index of filename  to id (1..n)
    std::stack<u16> m_openFilesStack;                 //id to get open file pointer from m_fileRecords
    std::vector<struct filerec> m_fileRecords;        //indexed by id ([0] is "Invalid" ~ NULL)

    FileManager * m_fileManager;                      //owner of this stream
    s32 m_lastReadByte;
    Locator m_lastReadLoc;

    CompilerState & m_state;

    // esa, 06072014 moved unread data members into filerec
    //               to be multi-file capable
    //Locator m_lastUnreadLoc;
    //bool m_haveUnreadByte;

    /** return false when stack is empty; o.w. true */
    bool discardTop();

    /** helper to get file id from path index in locator */
    bool getFileIdFromLocator(Locator loc, u32& idref);

    /** returns the accumulated line from the source */
    std::string getLineOfTextAsString(u32 id) const;

    /** updates the current line of text; if newline ships current
	line and current locator to CompilerState before clearing it;
	Locator has not been updated yet, so lines range from 0 to n-1. */
    void updateLineOfText(u32 id, char c);
  };

}

#endif  /* SOURCESTREAM_H */
