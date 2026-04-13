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
#include "config.h"

/** 
 * data buffer class, where data shall be stored.
 * a driver shall create its own buffers
 */
class DataBuffer {
  protected:
    nint size, index;
    double sampleRate;
    string name;
    bool inited;
  public:
    DataBuffer();
    virtual void init(nint len, double rate, string n);
    /**  function to write to the buffer and advance the needle
     * @param _data the data to write
     */
    virtual void write(void* sample);
    /** return the size
     */
    virtual nint getSize() const;
    /** return the needle
     * @return the position of the needle
     */
    virtual nint getIndex();
    /** return the sample rate
     * @return the sample rate
     */
    virtual double getSampleRate() const;
    /** return the buffer pointer
     * @return the address of the buffer
     */
    virtual void* getBuffer();
    /** return the name of the buffer
     * @return the name (as a std::string)
     */
    virtual string getName() const;
    /** destory the buffer - delete the buffer from memory and declare as not initialized
     */
    virtual void destroy();
    /** return the scaled buffer value (used in scopes)
     * @param i where
     * @return the value at that point, scaled to the range [-1...1]
     */
    virtual double getValueScaled(nint i);
    /** return the state of the buffer
     * @return the buffer state (true if initialized)
     */
    virtual bool isInited();
    /** save the buffer state to a node
     * @return a YAML::Node representing the buffer state
     */
    virtual YAML::Node writeToNode();
    /** load the buffer state from a node
     * @param node a node representing the buffer
     */
    virtual bool readFromNode(YAML::Node node);
    virtual ~DataBuffer();
};

class DataBuffer_Float : public DataBuffer {
  private:
    float* data;
  public:
    DataBuffer_Float();
    void init(nint len, double rate, string n);
    void write(void* sample);
    void* getBuffer();
    void destroy();
    double getValueScaled(nint i);
    ~DataBuffer_Float();
};
