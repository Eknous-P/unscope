/*
unscope - an audio oscilloscope
Copyright (C) 2025-2026 Eknous

unscope is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 2 of the License, or (at your option) any later
version.

unscope is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
unscope. If not, see <https://www.gnu.org/licenses/>. 
*/

#include "shared.h"
#include <cassert>

/** 
 * data buffer class, where data shall be stored.
 * a driver shall create its own buffers
 */
class DataBuffer {
  protected:
    nint size, index;
    double sampleRate;
    string name;
  public:
    virtual void init(nint len, double rate, string n) {
      size = len;
      sampleRate = rate;
      name = n;
      index = 0;
      assert(0 && "you called the virtual constructor!");
    }
    /**  function to write to the buffer and advance the needle
     * @param _data the data to write
     */
    virtual void write(void* sample) {
      (void)sample;
      if (index++>size) index=0;
    }
    /** return the size
     */
    virtual nint getSize() {
      return size;
    }
    /** return the needle
     * @return the position of the needle
     */
    virtual nint getIndex() {
      return index;
    }
    /** return the sample rate
     */
    virtual double getSampleRate() {
      return sampleRate;
    }
    /** return the buffer pointer
     */
    virtual void* getBuffer() {
      return NULL;
    }
    virtual string getName() {
      return name;
    }
    virtual void destroy() {
    }
    virtual double getValueScaled(nint i) {
      return 0.0;
    }
    virtual ~DataBuffer() {
    }
};

class DataBuffer_Float : public DataBuffer {
  private:
    float* data;
  public:
    DataBuffer_Float ():
      data(NULL) {}
    void init(nint len, double rate, string n) {
      size = len;
      sampleRate = rate;
      name = n;
      index = 0;
      data = new float[size];
    }
    void write(void* sample) {
      if (data==NULL) return;
      data[index++]=*(float*)sample;
      if (index>size) index=0;
    }
    void* getBuffer() {
      return data;
    }
    void destroy() {
      if (data) {
        delete[] data;
        data = NULL;
      }
    }
    double getValueScaled(nint i) {
      if (data==NULL) return 0;
      float s;
      if (index+i>size) {
        s=data[index+i-size];
      } else {
        s=data[index+i];
      }
      return s;
    }
    ~DataBuffer_Float() {
      if (data) {
        delete[] data;
        data = NULL;
      }
    }
};