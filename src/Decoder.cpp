#include <arpa/inet.h> // htons
#include <iomanip>
#include <sstream>

#include "Decoder.hpp"
#include "Markers.hpp"
#include "Utility.hpp"

namespace temp_peg
{

Decoder::Decoder()
{
    logFile << "Created \'Decoder object\'." << std::endl;
}
        
Decoder::Decoder(const std::string& filename)
{
    logFile << "Created \'Decoder object\'." << std::endl;
}

Decoder::~Decoder()
{
    close();
    logFile << "Destroyed \'Decoder object\'." << std::endl;
}

bool Decoder::open(const std::string& filename)
{
    m_imageFile.open(filename, std::ios::in | std::ios::binary);
    
    if (!m_imageFile.is_open() || !m_imageFile.good())
    {
        logFile << "Unable to open image: \'" + filename + "\'" << std::endl;
        return false;
    }
    
    logFile << "Opened JPEG image: \'" + filename + "\'" << std::endl;
    
    m_filename = filename;
    
    return true;
}

Decoder::ResultCode Decoder::decodeImageFile()
{
    if (!m_imageFile.is_open() || !m_imageFile.good())
    {
        logFile << "Unable scan image file: \'" + m_filename + "\'" << std::endl;
        return ResultCode::ERROR;
    }
    
    logFile << "Started decoding process..." << std::endl;
    
    UInt8 byte;
    ResultCode status = ResultCode::DECODE_DONE;
    
    while (m_imageFile >> std::noskipws >> byte)
    {
        if (byte == JFIF_BYTE_FF)
        {
            m_imageFile >> std::noskipws >> byte;
            
            ResultCode code = parseSegmentInfo(byte);
            
            if (code == ResultCode::SUCCESS)
                continue;
            else if (code == ResultCode::TERMINATE)
            {
                status = ResultCode::TERMINATE;
                break;
            }
            else if (code == ResultCode::DECODE_INCOMPLETE)
            {
                status = ResultCode::DECODE_INCOMPLETE;
                break;
            }
        }
        else
        {
            logFile << "[ FATAL ] Invalid JFIF file! Terminating..." << std::endl;
            status = ResultCode::ERROR;
            break;
        }
    }
    
    if (status == ResultCode::DECODE_DONE)
    {
        decodeScanData();
        m_image.createImageFromMCUs(m_MCU);
        logFile << "Finished decoding process [OK]." << std::endl;
    }
    else if (status == ResultCode::TERMINATE)
    {
        logFile << "Terminated decoding process [NOT-OK]." << std::endl;
    }
    
    else if (status == ResultCode::DECODE_INCOMPLETE)
    {
        logFile << "Decoding process incomplete [NOT-OK]." << std::endl;
    }
    
    return status;
}

bool Decoder::dumpRawData()
{
    std::size_t extPos = m_filename.find(".jpg");
    
    if (extPos == std::string::npos)
        extPos = m_filename.find(".jpeg");
    
    std::string targetFilename = m_filename.substr(0, extPos) + ".ppm";
    m_image.dumpRawData(targetFilename);
    
    return true;
}

void Decoder::close()
{
    m_imageFile.close();
    logFile << "Closed image file: \'" + m_filename + "\'" << std::endl;
}


void Decoder :: parseSOSSegment() {
        if (!m_imageFile.is_open() || !m_imageFile.good()) { // check if file is open and has proper integrity
            logFile << "Unable scan image file: \'" + m_filename + "\'" << std :: endl;
            return;
        }
        
        logFile << "Parsing SOS segment..." << std :: endl;
        
        UInt16 len;
        
        m_imageFile.read(reinterpret_cast<char *>(&len), 2);
        len = htons(len); // swaps the endianness of shorts (host-to-network short)
        
        logFile << "SOS segment length: " << len << std :: endl;
        
        UInt8 compCount; // Number of components
        UInt16 compInfo; // Component ID and Huffman table used
        
        m_imageFile >> std :: noskipws >> compCount; // load the value, do not skip white spaces
        
        if (compCount < 1 || compCount > 4) {
            logFile << "Invalid component count in image scan: " << (int)compCount << ", terminating decoding process..." << std :: endl;
            return;
        }
        
        logFile << "Number of components in scan data: " << (int)compCount << std :: endl;
        
        for (auto i = 0; i < compCount; ++i) {
            m_imageFile.read(reinterpret_cast<char *>(&compInfo), 2);
            compInfo = htons(compInfo); // swaps the endianness of shorts
            
            UInt8 cID = compInfo >> 8; // 1st byte denotes component ID 
            
            // 2nd byte denotes the Huffman table used:
            // Bits 7 to 4: DC Table #(0 to 3)
            // Bits 3 to 0: AC Table #(0 to 3)
            UInt8 DCTableNum = (compInfo & 0x00f0) >> 4;
            UInt8 ACTableNum = (compInfo & 0x000f);
            
            logFile << "Component ID: " << (int)cID << ", DC Table #: " << (int)DCTableNum << ", AC Table #: " << (int)ACTableNum << std :: endl;
        }
        
        // Skip the next three bytes
        for (auto i = 0; i < 3; ++i) {
            UInt8 byte;
            m_imageFile >> std :: noskipws >> byte;
        }
        
        logFile << "Finished parsing SOS segment [OK]" << std :: endl;
        
        scanImageData();
    }
    
void Decoder :: scanImageData() {
        if (!m_imageFile.is_open() || !m_imageFile.good()) {
            logFile << "Unable scan image file: \'" + m_filename + "\'" << std :: endl;
            return;
        }
        
        logFile << "Scanning image data..." << std :: endl;
        
        UInt8 byte;
        
        while (m_imageFile >> std :: noskipws >> byte) { // do not skip white spaces
            if (byte == JFIF_BYTE_FF) { // all the markers start with JFIF_BYTE_FF  as the MSB, defined in markers.hpp
                UInt8 prevByte = byte;
                
                m_imageFile >> std :: noskipws >> byte;
                
                if (byte == JFIF_EOI) {
                    logFile << "Found segment, End of Image (FFD9)" << std :: endl;
                    return;
                }
                
                std :: bitset<8> bits1(prevByte);
                logFile << "0x" << std :: hex << std :: setfill('0') << std :: setw(2)
                                            << std :: setprecision(8) << (int)prevByte
                                            << ", Bits: " << bits1 << std :: endl;
                                          
                m_scanData.append(bits1.to_string());
            }
            
            std :: bitset<8> bits(byte);
            logFile << "0x" << std :: hex << std :: setfill('0') << std :: setw(2)
                                        << std :: setprecision(8) << (int)byte
                                        << ", Bits: " << bits << std :: endl;
            
            m_scanData.append(bits.to_string());
        }
        
        logFile << "Finished scanning image data [OK]" << std :: endl;
}

// Parse the start of file segment

Decoder::ResultCode Decoder::parseSOF0Segment()
    {
                                                                       //Check whether image file is open or state of file is good.
        if (!m_imageFile.is_open() || !m_imageFile.good())      
        {
            logFile << "Unable scan image file: \'" + m_filename + "\'" << std::endl;
            return ResultCode::ERROR;
        }
        
        logFile << "Parsing SOF-0 segment..." << std::endl;
        
        UInt16 lenByte, imgHeight, imgWidth;
        UInt8 precision, compCount;
        
        m_imageFile.read(reinterpret_cast<char *>(&lenByte), 2);
        lenByte = htons(lenByte);                                //htons function converts values from host to network byte order
        
        logFile << "SOF-0 segment length: " << (int)lenByte << std::endl;       
        
        m_imageFile >> std::noskipws >> precision;                                      // It does not skip whitespace before next input and
        logFile << "SOF-0 segment data precision: " << (int)precision << std::endl;     // precision controlls the total number of digits that are printed.
        
        m_imageFile.read(reinterpret_cast<char *>(&imgHeight), 2);                      // image height and width
        m_imageFile.read(reinterpret_cast<char *>(&imgWidth), 2);
        
        imgHeight = htons(imgHeight);
        imgWidth = htons(imgWidth);
        
        logFile << "Image height: " << (int)imgHeight << std::endl;
        logFile << "Image width: " << (int)imgWidth << std::endl;
        
        m_imageFile >> std::noskipws >> compCount;                              //Extracting no. of components from image file
        
        logFile << "No. of components: " << (int)compCount << std::endl;            
        
        UInt8 compID = 0, sampFactor = 0, QTNo = 0;
        
        bool isNonSampled = true;
        
        for (auto i = 0; i < 3; ++i)
        {
            m_imageFile >> std::noskipws >> compID >> sampFactor >> QTNo;       // Extracting component ID ,sampling Factor and Quantization table of image
            
            logFile << "Component ID: " << (int)compID << std::endl;
            logFile << "Sampling Factor, Horizontal: " << int(sampFactor >> 4) << ", Vertical: " << int(sampFactor & 0x0F) << std::endl;
            logFile << "Quantization table no.: " << (int)QTNo << std::endl;
            
            if ((sampFactor >> 4) != 1 || (sampFactor & 0x0F) != 1)
                isNonSampled = false;
        }
        
        if (!isNonSampled)
        {
            logFile << "Chroma subsampling not yet supported!" << std::endl;
            logFile << "Chroma subsampling is not 4:4:4, terminating..." << std::endl;
            return ResultCode::TERMINATE;
        }
        
        logFile << "Finished parsing SOF-0 segment [OK]" << std::endl;        
        m_image.width = imgWidth;                                                   // Image height and width  in pixels
        m_image.height = imgHeight;
        
        return ResultCode::SUCCESS;
    }
    
//Parse the Huffman tables defined in the DHT segment.

void Decoder::parseDHTSegment()
{   
    //Check whether image file is open or state of file is good.
        
    if (!m_imageFile.is_open() || !m_imageFile.good())                          
    {
        logFile << "Unable scan image file: \'" + m_filename + "\'" << std::endl;
        return;
    }
        
    logFile << "Parsing Huffman table segment..." << std::endl;
        
    UInt16 len;
    m_imageFile.read(reinterpret_cast<char *>(&len), 2);
    len = htons(len);
        
        logFile << "Huffman table length: " << (int)len << std::endl;           //Prints huffman table length
        
        int segmentEnd = (int)m_imageFile.tellg() + len - 2;                    //Gives current position of get pointer
        
        while (m_imageFile.tellg() < segmentEnd)
        {
            UInt8 htinfo;
            m_imageFile >> std::noskipws >> htinfo;
            
            int HTType = int((htinfo & 0x10) >> 4);
            int HTNumber = int(htinfo & 0x0F);
            
            logFile << "Huffman table type: " << HTType << std::endl;
            logFile << "Huffman table #: " << HTNumber << std::endl;
            
            int totalSymbolCount = 0;
            UInt8 symbolCount;
                                                                                //Count total number of symbols
            for (auto i = 1; i <= 16; ++i)
            {
                m_imageFile >> std::noskipws >> symbolCount;
                m_huffmanTable[HTType][HTNumber][i-1].first = (int)symbolCount;
                totalSymbolCount += (int)symbolCount;
            }
            
                                                                                // Load the symbols
            int syms = 0;
            for (auto i = 0; syms < totalSymbolCount; )
            {
                /* Read the next symbol, and add it to the proper slot in the Huffman table.
                 Depndending upon the symbol count, say n, for the current
                 symbol length, insert the next n symbols in the symbol
                 list to it's proper spot in the Huffman table. This means,
                 if symbol counts for symbols of lengths 1, 2 and 3 are 0,
                 5 and 2 respectively, the symbol list will contain 7
                 symbols, out of which the first 5 are symbols with length
                 2, and the remaining 2 are of length 3. */
                
                UInt8 code;
                m_imageFile >> std::noskipws >> code;
                
                if (m_huffmanTable[HTType][HTNumber][i].first == 0)
                {
                    while (m_huffmanTable[HTType][HTNumber][++i].first == 0);
                }   
                
                m_huffmanTable[HTType][HTNumber][i].second.push_back(code);
                syms++;
                
                if (m_huffmanTable[HTType][HTNumber][i].first == m_huffmanTable[HTType][HTNumber][i].second.size())
                    i++;
            }
            
            logFile << "Printing symbols for Huffman table (" << HTType << "," << HTNumber << ")..." << std::endl;
            
            int totalCodes = 0;
            for (auto i = 0; i < 16; ++i)
            {
                std::string codeStr = "";
                for (auto&& symbol : m_huffmanTable[HTType][HTNumber][i].second)
                {
                    std::stringstream ss;
                    //setfill()-Sets charecter specified 
                    //setw() - sets the number of charecter to be used
                    //setprecision() - set a decimal precision
                    ss << "0x" << std::hex << std::setfill('0') << std::setw(2) << std::setprecision(16) << (int)symbol;
                    codeStr += ss.str() + " ";
                    totalCodes++;
                }
                
                logFile << "Code length: " << i+1
                                        << ", Symbol count: " << m_huffmanTable[HTType][HTNumber][i].second.size()
                                        << ", Symbols: " << codeStr << std::endl;
            }
            
            logFile << "Total Huffman codes for Huffman table(Type:" << HTType << ",#:" << HTNumber << "): " << totalCodes << std::endl;
            
            m_huffmanTree[HTType][HTNumber].constructHuffmanTree(m_huffmanTable[HTType][HTNumber]);
            auto htree = m_huffmanTree[HTType][HTNumber].getTree();                     //Get root node of huffman tree
            logFile << "Huffman codes:-" << std::endl;
            inOrder(htree);                                                             // Output : Inorder Traveral of a huffman tree
        }
        
        logFile << "Finished parsing Huffman table segment [OK]" << std::endl;
    }


/*
In the jpeg decoders, the byte "ff" has special meaning
so something like ff d8 would be the SOI segment 
but when this same marker byte occurs anywhere else in the file
it cann get misinterpreted, so to avoid that misinterpretation,
we insert the empty byte "00".
This method does that.
*/
void Decoder::byteStuffScanData()
{
    if (m_scanData.empty())
    {
        logFile << " [ FATAL ] Invalid image scan data" << std::endl;
        return;
    }
    
    logFile << "Byte stuffing image scan data..." << std::endl;
    
    for (unsigned i = 0; i <= m_scanData.size() - 8; i += 8)
    {
        std::string byte = m_scanData.substr(i, 8);
        
        if (byte == "11111111")
        {
            if (i + 8 < m_scanData.size() - 8)
            {
                std::string nextByte = m_scanData.substr(i + 8, 8);
                
                if (nextByte == "00000000")
                {
                    m_scanData.erase(i + 8, 8);
                }
                else continue;
            }
            else
                continue;
        }
    }
    
    logFile << "Finished byte stuffing image scan data [OK]" << std::endl;
}


void Decoder::decodeScanData()
{
    if (m_scanData.empty())
    {
        logFile << " [ FATAL ] Invalid image scan data" << std::endl;
        return;
    }
    
    byteStuffScanData();
    
    logFile << "Decoding image scan data..." << std::endl;
    
    const char* component[] = { "Y (Luminance)", "Cb (Chrominance)", "Cr (Chrominance)" };
    const char* type[] = { "DC", "AC" };        
    
    // get number of MCUs in the image file
    int MCUCount = (m_image.width * m_image.height) / 64;
    
    m_MCU.clear();
    logFile << "MCU count: " << MCUCount << std::endl;
    
    int k = 0; // The index of the next bit to be scanned
    
    /*
    Decoding the bit stream for each MCU
    */
    for (auto i = 0; i < MCUCount; ++i)
    {
        logFile << "Decoding MCU-" << i + 1 << "..." << std::endl;
        
        // The run-length coding after decoding the Huffman data
        std::array<std::vector<int>, 3> RLE;            
        
        // For each component Y, Cb & Cr, decode 1 DC
        // coefficient and then decode 63 AC coefficients.
        //
        // NOTE:
        // Since a signnificant portion of a RLE for a
        // component contains a trail of 0s, AC coefficients
        // are decoded till, either an EOB (End of block) is
        // encountered or 63 AC coefficients have been decoded.
        
        for (auto compID = 0; compID < 3; ++compID)
        {
            std::string bitsScanned = ""; // Initially no bits are scanned
            
            // Firstly, decode the DC coefficient
            logFile << "Decoding MCU-" << i + 1 << ": " << component[compID] << "/" << type[HT_DC] << std::endl;
            
            int HuffTableID = compID == 0 ? 0 : 1;
            
            while (1)
            {       
                bitsScanned += m_scanData[k];
                auto value = m_huffmanTree[HT_DC][HuffTableID].contains(bitsScanned);
                
                if (!utils::isStringWhiteSpace(value))
                {
                    if (value != "EOB")
                    {   
                        int zeroCount = UInt8(std::stoi(value)) >> 4 ;
                        int category = UInt8(std::stoi(value)) & 0x0F;
                        int DCCoeff = bitStringtoValue(m_scanData.substr(k + 1, category));
                        
                        k += category + 1;
                        bitsScanned = "";
                        
                        RLE[compID].push_back(zeroCount);
                        RLE[compID].push_back(DCCoeff);
                        
                        break;
                    }
                    
                    else
                    {
                        bitsScanned = "";
                        k++;
                        
                        RLE[compID].push_back(0);
                        RLE[compID].push_back(0);
                        
                        break;
                    }
                }
                else
                    k++;
            }
            
            // Then decode the AC coefficients
            logFile << "Decoding MCU-" << i + 1 << ": " << component[compID] << "/" << type[HT_AC] << std::endl;
            bitsScanned = "";
            int ACCodesCount = 0;
                            
            while (1)
            {   
                // If 63 AC codes have been encountered, this block is done, move onto next block                    
                if (ACCodesCount  == 63)
                {
                    break;
                }
                
                // Append the k-th bit to the bits scanned so far
                bitsScanned += m_scanData[k];
                auto value = m_huffmanTree[HT_AC][HuffTableID].contains(bitsScanned);
                
                if (!utils::isStringWhiteSpace(value))
                {
                    if (value != "EOB")
                    {
                        int zeroCount = UInt8(std::stoi(value)) >> 4 ;
                        int category = UInt8(std::stoi(value)) & 0x0F;
                        int ACCoeff = bitStringtoValue(m_scanData.substr(k + 1, category));
                        
                        k += category + 1;
                        bitsScanned = "";
                        
                        RLE[compID].push_back(zeroCount);
                        RLE[compID].push_back(ACCoeff);
                        
                        ACCodesCount += zeroCount + 1;
                    }
                    
                    else
                    {
                        bitsScanned = "";
                        k++;
                        
                        RLE[compID].push_back(0);
                        RLE[compID].push_back(0);
                        
                        break;
                    }
                }
                
                else
                    k++;
            }
            
            // If both the DC and AC coefficients are EOB, truncate to (0,0)
            if (RLE[compID].size() == 2)
            {
                bool allZeros = true;
                
                for (auto&& rVal : RLE[compID])
                {
                    if (rVal != 0)
                    {
                        allZeros = false;
                        break;
                    }
                }
                
                // Remove the extra (0,0) pair
                if (allZeros)
                {
                    RLE[compID].pop_back();
                    RLE[compID].pop_back();
                }
            }
        }
        
        // Construct the MCU block from the RLE &
        // quantization tables to a 8x8 matrix
        m_MCU.push_back(MCU(RLE, m_QTables));
        
        logFile << "Finished decoding MCU-" << i + 1 << " [OK]" << std::endl;
    }
    
    // The remaining bits, if any, in the scan data are discarded as
    // they're added byte align the scan data.
    
    logFile << "Finished decoding image scan data [OK]" << std::endl;
}


}