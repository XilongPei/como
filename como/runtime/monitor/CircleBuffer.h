//=========================================================================
// Copyright (C) 2022 The C++ Component Model(COMO) Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//=========================================================================

// https://www.cnblogs.com/-citywall123/p/12858865.html

#ifndef __CIRCLE_BUFFER_H__
#define __CIRCLE_BUFFER_H__

#include <assert.h>
#include <stdlib.h>
#include <string.h>

namespace como {

template<typename T>
class CircleBuffer
{
public:
    CircleBuffer(size_t size)
    {
        m_nBufSize = size;
        m_nReadPos = 0;
        m_nWritePos = 0;
        m_pBuf = new T[m_nBufSize];
        m_bEmpty = true;
        m_bFull = false;
    }

    ~CircleBuffer()
    {
        if (m_pBuf) {
            delete[] m_pBuf;
            m_pBuf = nullptr;
        }
    }

    bool isFull()
    {
        return m_bFull;
    }

    bool isEmpty()
    {
        return m_bEmpty;
    }

    void Clear()
    {
        m_nReadPos = 0;
        m_nWritePos = 0;
        m_bEmpty = true;
        m_bFull = false;
    }

    int GetLength()
    {
        if (m_bEmpty) {
            return 0;
        }
        else if (m_bFull) {
            return m_nBufSize;
        }
        else if (m_nReadPos < m_nWritePos) {
            return m_nWritePos - m_nReadPos;
        }
        else {
            return m_nBufSize - m_nReadPos + m_nWritePos;
        }
    }

    int Write(T* buf, int count)
    {
        if (count <= 0)
            return 0;

        m_bEmpty = false;

        if (m_bFull) {
            return 0;
        }

        // When the buffer is empty
        if (m_nReadPos == m_nWritePos) {
            /* memory model
                    (empty)                    m_nReadPos                (empty)
             |-----------------------|-----------------------------------------|
                                 m_nWritePos                             m_nBufSize
             */
            // Calculate the remaining writable space
            int leftcount = m_nBufSize - m_nWritePos;
            if (leftcount > count) {
                memcpy(m_pBuf + m_nWritePos, buf, count);
                // Move tail position
                m_nWritePos += count;
                m_bFull = (m_nWritePos == m_nReadPos);
                return count;
            }
            else {
                // Put the data that can be put into the buffer first
                memcpy(m_pBuf + m_nWritePos, buf, leftcount);

                // Move the write pointer. If the area on the right of the write
                // pointer can put down the remaining data, it will be offset to
                // the `cont-leftcount` position, Otherwise, it will be shifted
                // to the read pointer position, indicating that the buffer is
                // full and the redundant data will be discarded.
                m_nWritePos = (m_nReadPos > count - leftcount) ?
                                                count - leftcount : m_nWritePos;

                // Open up new memory along the end and write the remaining data
                memcpy(m_pBuf, buf + leftcount, m_nWritePos);
                m_bFull = (m_nWritePos == m_nReadPos);

                return leftcount + m_nWritePos;
            }
        }
        // There is space left, and the write pointer is in front of the read pointer
        else if (m_nReadPos < m_nWritePos) {
            /* memory model
             (empty)                        (data)                  (empty)
             |-------------------|------------------------|--------------------|
                            m_nReadPos                m_nWritePos       (leftcount)
             */
            // Calculate the remaining buffer size (from the write position to
            // the end of the buffer)
            int leftcount = m_nBufSize - m_nWritePos;
            if (leftcount > count) {  // Space is enough
                // Write into buffer
                memcpy(m_pBuf + m_nWritePos, buf, count);

                m_nWritePos += count;
                m_bFull = (m_nReadPos == m_nWritePos);
                assert(m_nReadPos <= m_nBufSize);
                assert(m_nWritePos <= m_nBufSize);
                return count;
            }
            // There is not enough space left on the right side of the write
            // pointer to put down the data
            else {
                // First fill up the remaining space on the right side of the
                // write pointer, and then see if the remaining data can be put
                // down on the left side of the read pointer
                memcpy(m_pBuf + m_nWritePos, buf, leftcount);

                // Move the write pointer. If the area on the left of the read
                // pointer can put down the remaining data, it will be offset to
                // the `cont - leftcount` position, Otherwise, it will be shifted
                // to the read pointer position, indicating that the cache is
                // full and the redundant data will be discarded.
                m_nWritePos = (m_nReadPos >= count - leftcount) ?
                                                 count - leftcount : m_nReadPos;

                // Open up new memory along the end and write the remaining data
                memcpy(m_pBuf, buf + leftcount, m_nWritePos);
                m_bFull = (m_nReadPos == m_nWritePos);
                assert(m_nReadPos <= m_nBufSize);
                assert(m_nWritePos <= m_nBufSize);
                return leftcount + m_nWritePos;
            }
        }
        // The read pointer precedes the write pointer
        else {
            /* memory model
             (unread)                 (read)                     (unread)
             |-------------------|----------------------------|----------------|
                            m_nWritePos        (leftcount)    m_nReadPos
             */
            int leftcount = m_nReadPos - m_nWritePos;
            if (leftcount > count) {
                // There is enough spare space for storage
                memcpy(m_pBuf + m_nWritePos, buf, count);
                m_nWritePos += count;
                m_bFull = (m_nReadPos == m_nWritePos);
                assert(m_nReadPos <= m_nBufSize);
                assert(m_nWritePos <= m_nBufSize);
                return count;
            }
            else {
                // When the remaining space is insufficient, the following data
                // shall be discarded
                memcpy(m_pBuf + m_nWritePos, buf, leftcount);
                m_nWritePos += leftcount;
                m_bFull = (m_nReadPos == m_nWritePos);
                assert(m_bFull);
                assert(m_nReadPos <= m_nBufSize);
                assert(m_nWritePos <= m_nBufSize);
                return leftcount;
            }
        }
    }

    int Read(T* buf, int count)
    {
        if (count <= 0)
            return 0;

        m_bFull = false;

        if (m_bEmpty) {
            return 0;
        }

        // When buffer if full
        if (m_nReadPos == m_nWritePos) {
            /* memory model
                    (data)                    m_nReadPos                (data)
             |------------------------|----------------------------------------|
                                 m_nWritePos         m_nBufSize
             */
            int leftcount = m_nBufSize - m_nReadPos;
            if (leftcount > count) {
                memcpy(buf, m_pBuf + m_nReadPos, count);
                m_nReadPos += count;
                m_bEmpty = (m_nReadPos == m_nWritePos);
                return count;
            }
            else {
                memcpy(buf, m_pBuf + m_nReadPos, leftcount);
                // If the area to the left of the write pointer can read out the
                // remaining data, it will be shifted to the `count - leftcount`
                // position. Otherwise, it will be offset to the write pointer
                // position, indicating that the cache is empty
                m_nReadPos = (m_nWritePos > count - leftcount) ?
                                                count - leftcount : m_nWritePos;
                memcpy(buf + leftcount, m_pBuf, m_nReadPos);
                m_bEmpty = (m_nReadPos == m_nWritePos);
                return leftcount + m_nReadPos;
            }
        }
        // Write pointer before (unread data is continuous)
        else if (m_nReadPos < m_nWritePos) {
            /* memory model
             (read)                 (unread)                      (read)
             |-------------------|------------------------|--------------------|
                            m_nReadPos                m_nWritePos         m_nBufSize
             */
            int leftcount = m_nWritePos - m_nReadPos;
            int icount = (leftcount > count) ? count : leftcount;
            memcpy(buf, m_pBuf + m_nReadPos, icount);
            m_nReadPos += icount;
            m_bEmpty = (m_nReadPos == m_nWritePos);
            assert(m_nReadPos <= m_nBufSize);
            assert(m_nWritePos <= m_nBufSize);
            return icount;
        }
        else {
            /* memory model
             (unread)                (read)                      (unread)
             |-------------------|--------------------|----_-------------------|
                            m_nWritePos           m_nReadPos              m_nBufSize

             */
            int leftcount = m_nBufSize - m_nReadPos;
            // The data to be read out is continuous and on the right of the
            // reading pointer m_nReadPos<=count<=m_nBufSize
            if (leftcount > count)
            {
                memcpy(buf, m_pBuf + m_nReadPos, count);
                m_nReadPos += count;
                m_bEmpty = (m_nReadPos == m_nWritePos);
                assert(m_nReadPos <= m_nBufSize);
                assert(m_nWritePos <= m_nBufSize);
                return count;
            }
            // The data to be read out is discontinuous, which is on the right
            // of the read pointer and the left of the write pointer respectively
            else {
                // First read the data to the right of the reading pointer
                memcpy(buf, m_pBuf + m_nReadPos, leftcount);

                // Move the position of the read pointe. If the area to the left
                // of the write pointer can read out the remaining data, it will
                // be offset to the `cont-leftcount` position,
                // Otherwise, it will be offset to the write pointer position,
                // indicating that the cache is empty and the redundant data
                // will be discarded
                m_nReadPos = (m_nWritePos >= count - leftcount) ?
                                                count - leftcount : m_nWritePos;

                // Read the data to the left of the write pointer
                memcpy(buf, m_pBuf, m_nReadPos);
                m_bEmpty = (m_nReadPos == m_nWritePos);
                assert(m_nReadPos <= m_nBufSize);
                assert(m_nWritePos <= m_nBufSize);
                return leftcount + m_nReadPos;
            }
        }
    }

    int GetReadPos()
    {
        return m_nReadPos;
    }

    int GetWritePos()
    {
        return m_nWritePos;
    }

private:
    bool m_bEmpty, m_bFull;
    T * m_pBuf = nullptr;
    size_t m_nBufSize;
    int m_nReadPos;
    int m_nWritePos;

};

} // namespace como

#endif // __CIRCLE_BUFFER_H__
