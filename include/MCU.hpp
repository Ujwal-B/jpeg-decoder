<<<<<<< HEAD
#ifndef MCU_HPP
#define MCU_HPP

#include <array>
#include <vector>
#include <utility>

#include "Types.hpp"
#include "Transform.hpp"

namespace temp_peg
{

// Alias for a 8x8 pixel block with integral values for its channels
typedef std::array<std::array<std::array<int, 8>, 8>, 3> CompMatrices;

// Alias for a 8x8 matrix with integral elements
typedef std::array< std::array< int, 8 >, 8 > Matrix8x8;

class MCU
{
public:
    
    static int m_MCUCount;
    // m_QTables is the quantization tables used for quantization
    static std::vector<std::vector<UInt16>> m_QTables;

public:
    
    MCU();
    
    MCU(const std::array<std::vector<int>, 3>& compRLE,
        const std::vector<std::vector<UInt16>>& QTables);
    
    void constructMCU(const std::array<std::vector<int>, 3>& compRLE,
                        const std::vector<std::vector<UInt16>>& QTables);
    
    const CompMatrices& getAllMatrices() const;

private:
    
    void computeIDCT();
    
    void performLevelShift();
    
    void convertYCbCrToRGB();
    
private:
    
    CompMatrices m_block;
    int m_order;
    static int m_DCDiff[3];
    std::array<std::array<std::array<float, 8>, 8>, 3> m_IDCTCoeffs;
};

}
#endif //MCU HPP
=======
/* 
Minimum Coded Unit (MCU) module

Represents an abstraction for a MCU. An MCU is a block of 8x8 pixels.
This abstracts away the conversion of the decoded RLE-Huffman encoded pixel values
to corresponding matrices of 8x8 size. It actually contains three 8x8 matrices to
represent the lumninance (Y) and chrominance (Cb & Cr) components.
 
The MCU object expects as input the RLE-Huffman encoded vector (obtained after
the Huffman decoding is done).
*/

#ifndef MCU_HPP // only if MCU_HPP is defined then the code in #ifndef and #endif will be taken for consideration
#define MCU_HPP

#include <array>
#include <vector>
#include <utility>

#include "Types.hpp" // types module for aliases
#include "Transform.hpp" // transform module for zizzag and matrice indices transformation

using namespace std;

namespace temp_peg {
    // alias for a 8x8 pixel block with integral values for its channels
    typedef array <array <array <int, 8>, 8>, 3> CompMatrices;
    
    // alias for a 8x8 matrix with integral elements
    typedef array <array <int, 8>, 8> Matrix8x8;
    
    class MC {
        public:
            
            // The total number of MCUs in the image
            static int m_MCUCount;
            
            // The quantization table used for de-quantization
            static vector <vector <UInt16> > m_QTables;

        public:
            
            // Default constructor
            MCU();
            
            // Parameterized constructor
            // Initialze the MCU with run-length encoding per
            // channel and the quantization tables to use
            // parameter compRLE: the run-length encoding for the MCU
            // parameter QTables: the quantization tables to be used for encoding the MCU
            MCU(const array <vector <int>, 3> &compRLE, const vector <vector <UInt16> > &QTables);
            
            // Create the MCU from the specified run-length encoding and quantization tables
            // parameter compRLE: the run-length encoding for the MCU
            // parameter QTables: the quantization tables to be used for encoding the MCU
            void constructMCU(const array<vector<int>, 3> &compRLE const vector <vector <UInt16> > &QTables);
            
            // Get the pixel arrays for the pixels under this MCU.
            // Since there are three channels per MCU, three pixel arrays will be returned.
            const CompMatrices& getAllMatrices() const;
        
        private:
            
            // Inverse discrete cosine transform (IDCT)
            // The 8x8 matrices for each component has to be converted
            // back from frequency to spaital domain.
            void computeIDCT();
            
            // Level shift the pixel data to center it within the pixel value range
            void performLevelShift();
            
            // Convert the MCU's underlying pixels from the Y-Cb-Cr color model to RGB color model
            void convertYCbCrToRGB();
            
        private:
            
            // The pixel arrays for the three channels in the MCU
            CompMatrices m_block;

            // The order of the MCU in the image
            int m_order;
            
            // The differences in the DC coefficients per channel
            static int m_DCDiff[3];
            
            // For storing the IDCT coefficients before level shifting
            array <array <array <float, 8>, 8>, 3> m_IDCTCoeffs;
    };
}

#endif // MCU_HPP
>>>>>>> 7f42acb8ac39a89f2161a5c99cbd48694c806235
