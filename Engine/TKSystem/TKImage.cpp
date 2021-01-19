#include "TKImage.h"
USING_TK_NAMESPACE

TCHAR TKImage::ms_ImageFormat[IF_MAX][10] =
{
	_T("bmp"),
	_T("tga")
};

TKImage::TKImage()
{
    m_uiWidth = 0;
    m_uiHeight = 0;
    m_uiBPP = 0;
    m_pImageData = nullptr;
    m_pData = nullptr;
    m_pPalette = nullptr;
}

TKImage::~TKImage()
{
    TKMAC_DELETE(m_pImageData);
    TKMAC_DELETE(m_pData);
    TKMAC_DELETE(m_pPalette);
}

// =========================TKBMPImage Begin=============================
TKBMPImage::TKBMPImage()
{
    m_uiEnc = 0;
    m_uiPlanes = 0;
    m_uiImgOffset = 0;
}

TKBMPImage::~TKBMPImage()
{
}

bool TKBMPImage::Load(const TCHAR * pFileName)
{
    TKMAC_DELETE(m_pImageData);
    TKMAC_DELETE(m_pData);
    TKMAC_DELETE(m_pPalette);
    if (!GetFile(pFileName))
        return false;


    if (!ReadBmpHeader())
        return false;

    switch (m_uiEnc)
    {
        case 0: // Uncompressed
            if (!LoadBmpRaw())
                return false;
            break;

        case 1: // RLE 8 (Indexed 256 colour only)
            if (!LoadBmpRLE8())
                return false;
            break;

        case 2: // RLE 4 (16 Colour indexed, Outdated, not covered here)
            return false;

        case 3: // Bitfields (16/32 bit only, Rare, not covered here)
            return false;

        default:
            return false;
    }

    // Flip image to correct scanline reversal
    FlipImg();

    // Load palette if present
    if (!LoadBmpPalette())
        return false;

    // Free the file data
    TKMAC_DELETEA(m_pData);

    return true;
}

bool TKBMPImage::LoadFormBuffer(unsigned char * pBuffer, unsigned int uiSize)
{
    if (!pBuffer)
    {
        return false;
    }
    TKMAC_DELETEA(m_pImageData);
    TKMAC_DELETEA(m_pPalette);

    m_pData = NULL;

    m_pData = pBuffer;

    if (!ReadBmpHeader())
        return false;

    switch (m_uiEnc)
    {
        case 0: // Uncompressed
            if (!LoadBmpRaw())
                return false;
            break;

        case 1: // RLE 8 (Indexed 256 colour only)
            if (!LoadBmpRLE8())
                return false;
            break;

        case 2: // RLE 4 (16 Colour indexed, Outdated, not covered here)
            return false;

        case 3: // Bitfields (16/32 bit only, Rare, not covered here)
            return false;

        default:
            return false;
    }


    // Flip image to correct scanline reversal
    FlipImg();

    // Load palette if present
    if (!LoadBmpPalette())
        return false;

    // Free the file data
    m_pData = NULL;
    return true;
}

const unsigned char * TKBMPImage::GetPixel(unsigned int x, unsigned int y) const
{
    if (x < 0) x = 0;
    if (x >= m_uiWidth) x = m_uiWidth - 1;
    if (y < 0) y = 0;
    if (y >= m_uiHeight) y = m_uiHeight - 1;

    int bytes_per_pixel = m_uiBPP / 8;
    int loc = (y * m_uiWidth + x) * bytes_per_pixel;

    return m_pImageData + loc;
}


bool TKBMPImage::GetFile(const TCHAR * pFilename)
{
    TKFile ImageFile;
    ImageFile.Open(pFilename, TKFile::OM_RB);
    if (!ImageFile.GetFileSize())
    {
        return false;
    }
    m_pData = TK_NEW unsigned char[ImageFile.GetFileSize()];
    if (!m_pData)
        return false;

    ImageFile.Read(m_pData, ImageFile.GetFileSize(), 1);
    return true;
}

bool TKBMPImage::ReadBmpHeader()
{
    int iInfo;
    if (m_pData[0x0] != 'B' || m_pData[0x1] != 'M') // BMP ID Bytes, should be 'BM'
        return false;

    TKMemcpy(&m_uiImgOffset, &m_pData[0xA], 4);   // Offset to image data
    TKMemcpy(&iInfo, &m_pData[0xE], 4);        // Info header size, should be 0x28

    if (iInfo != 0x28)
        return false;

    TKMemcpy(&m_uiWidth, &m_pData[0x12], 4);   // Image width
    TKMemcpy(&m_uiHeight, &m_pData[0x16], 4);  // Image height
    TKMemcpy(&m_uiPlanes, &m_pData[0x1A], 2);  // Colour planes
    TKMemcpy(&m_uiBPP, &m_pData[0x1C], 2);     // BPP
    TKMemcpy(&m_uiEnc, &m_pData[0x1E], 4);     // Encoding

    m_uiDataSize = (m_uiWidth * m_uiHeight * (m_uiBPP >> 3)); // Calculate Image Data size
    return true;
}

bool TKBMPImage::LoadBmpRaw()
{
    // Allocate space for the image data
    m_pImageData = TK_NEW unsigned char[m_uiDataSize];
    if (!m_pImageData)
        return false;

    TKMemcpy(m_pImageData, &m_pData[m_uiImgOffset], m_uiDataSize);
    return true;
}

bool TKBMPImage::LoadBmpRLE8()
{
    unsigned char bOpCode, bVal;
    unsigned char *pSrc;
    unsigned int iDcode = 1, iCount, iPos, iIndex;

    m_pImageData = TK_NEW unsigned char[m_uiDataSize];
    if (!m_pImageData)
        return false;

    // Get the start of the RLE data
    pSrc = &m_pData[m_uiImgOffset];

    iPos = 0;
    iIndex = 0;

    while (iDcode)
    {
        // Stay on even bytes
        while (iPos % 2)
        {
            iPos++;
        }

        bOpCode = pSrc[iPos];
        bVal = pSrc[iPos + 1];
        iPos += 2;

        if (bOpCode > 0) // Run mode, Repeat 'bVal' 'OpCode' times
        {
            for (iCount = 0; iCount != bOpCode; iCount++)
            {
                m_pImageData[iIndex] = bVal;
                ++iIndex;
            }
        }
        else // Absolute Mode (Opcode=0), various options
        {
            switch (bVal)
            {
                case 0:  // EOL, no action
                    break;

                case 1:  // EOF, STOP!
                    iDcode = 0;
                    break;

                case 2:  // Reposition, Never used
                    break;

                default: // Copy the next 'bVal' bytes directly to the image
                    for (iCount = bVal; iCount != 0; iCount--)
                    {
                        m_pImageData[iIndex] = pSrc[iPos];
                        ++iIndex;
                        ++iPos;
                    }
                    break;
            }
        }

        if (iIndex > m_uiDataSize) // Stop if image size exceeded.
            iDcode = 0;
    }

    return true;
}

bool TKBMPImage::LoadBmpPalette()
{
    int iIndex;
    unsigned char *pPalPos, *pDatPos;

    if (m_uiBPP > 8) // NULL Palette for RGB images
        return true;

    // Create space for palette
    m_pPalette = TK_NEW unsigned char[768];

    if (!m_pPalette)
        return false;

    // Set starting position for pointers
    pPalPos = m_pPalette;
    pDatPos = &m_pData[0x36];

    // Get colour values, skip redundant 4th value
    for (iIndex = 0; iIndex != 256; ++iIndex)
    {
        pPalPos[0] = pDatPos[2]; // Red
        pPalPos[1] = pDatPos[1]; // Green
        pPalPos[2] = pDatPos[0]; // Blue

        pPalPos += 3;
        pDatPos += 4;
    }

    return true;
}

void TKBMPImage::FlipImg()
{
    unsigned char bTemp;
    unsigned char *pLine1, *pLine2;
    unsigned iLineLen, iIndex;

    iLineLen = m_uiWidth * (m_uiBPP / 8);
    pLine1 = m_pImageData;
    pLine2 = &m_pImageData[iLineLen * (m_uiHeight - 1)];

    for (; pLine1 < pLine2; pLine2 -= (iLineLen * 2))
    {
        for (iIndex = 0; iIndex != iLineLen; pLine1++, pLine2++, iIndex++)
        {
            bTemp = *pLine1;
            *pLine1 = *pLine2;
            *pLine2 = bTemp;
        }
    }
}
// =========================TKBMPImage End===============================

// =========================TKTGAImage Begin=============================
TKTGAImage::TKTGAImage()
{
    m_ucEnc = 0;
}

TKTGAImage::~TKTGAImage()
{
}

bool TKTGAImage::Load(const TCHAR * pFileName)
{
    unsigned long ulSize;

    // Clear out any existing image and palette
    TKMAC_DELETEA(m_pImageData);
    TKMAC_DELETEA(m_pPalette);
    TKMAC_DELETEA(m_pData);

    // Open the specified file
    TKFile ImageFile;
    if (!ImageFile.Open(pFileName, TKFile::OM_RB))
        return false;

    ulSize = ImageFile.GetFileSize();
    if (!ulSize)
        return false;

    m_pData = TK_NEW unsigned char[ulSize];
    if (!m_pData)
        return false;

    // Read the file into memory
    ImageFile.Read(m_pData, ulSize, 1);

    // Process the header
    if (!ReadHeader())
        return false;

    switch (m_ucEnc)
    {
        case 1: // Raw Indexed
        {
            // Check filesize against header values
            if ((m_uiDataSize + 18 + m_pData[0] + 768) > ulSize)
                return false;

            // Double check image type field
            if (m_pData[1] != 1)
                return false;

            // Load image data
            if (!LoadRawData())
                return false;

            if (!LoadTgaPalette())
                return false;

            break;
        }

        case 2: // Raw RGB
        {
            // Check filesize against header values
            if ((m_uiDataSize + 18 + m_pData[0]) > ulSize)
                return false;

            // Double check image type field
            if (m_pData[1] != 0)
                return false;

            // Load image data
            if (!LoadRawData())
                return false;

            //BGRtoRGB(); // Convert to RGB
            break;
        }

        case 9: // RLE Indexed
        {
            // Double check image type field
            if (m_pData[1] != 1)
                return false;

            // Load image data
            if (!LoadTgaRLEData())
                return false;

            // Load palette
            if (!LoadTgaPalette())
                return false;

            break;
        }

        case 10: // RLE RGB
        {
            // Double check image type field
            if (m_pData[1] != 0)
                return false;

            // Load image data
            if (!LoadTgaRLEData())
                return false;

            //BGRtoRGB(); // Convert to RGB
            break;
        }

        default:
        {
            // Check filesize against header values
            if ((m_uiDataSize + 18 + m_pData[0]) > ulSize)
                return false;

            // Double check image type field
            if (m_pData[1] != 0)
                return false;

            // Load image data
            if (!LoadRawData())
                return false;

            //BGRtoRGB(); // Convert to RGB
            break;
        }

    }

    // Check flip bit
    if ((m_pData[17] & 0x20) == 0)
        FlipImg();

    // Release file memory
    TKMAC_DELETEA(m_pData);
    return true;
}

bool TKTGAImage::LoadFormBuffer(unsigned char * pBuffer, unsigned int uiSize)
{
    if (!pBuffer)
    {
        return false;
    }
    // Clear out any existing image and palette
    TKMAC_DELETEA(m_pImageData);
    TKMAC_DELETEA(m_pPalette);
    TKMAC_DELETEA(m_pData);

    m_pData = pBuffer;
    // Process the header
    if (!ReadHeader())
        return false;

    switch (m_ucEnc)
    {
        case 1: // Raw Indexed
        {
            // Check filesize against header values
            if ((m_uiDataSize + 18 + m_pData[0] + 768) > uiSize)
                return false;

            // Double check image type field
            if (m_pData[1] != 1)
                return false;

            // Load image data
            if (!LoadRawData())
                return false;

            if (!LoadTgaPalette())
                return false;

            break;
        }

        case 2: // Raw RGB
        {
            // Check filesize against header values
            if ((m_uiDataSize + 18 + m_pData[0]) > uiSize)
                return false;

            // Double check image type field
            if (m_pData[1] != 0)
                return false;

            // Load image data
            if (!LoadRawData())
                return false;

            //BGRtoRGB(); // Convert to RGB
            break;
        }

        case 9: // RLE Indexed
        {
            // Double check image type field
            if (m_pData[1] != 1)
                return false;

            // Load image data
            if (!LoadTgaRLEData())
                return false;

            // Load palette
            if (!LoadTgaPalette())
                return false;

            break;
        }

        case 10: // RLE RGB
        {
            // Double check image type field
            if (m_pData[1] != 0)
                return false;

            // Load image data
            if (!LoadTgaRLEData())
                return false;

            //BGRtoRGB(); // Convert to RGB
            break;
        }

        default:
        {
            // Check filesize against header values
            if ((m_uiDataSize + 18 + m_pData[0]) > uiSize)
                return false;

            // Double check image type field
            if (m_pData[1] != 0)
                return false;

            // Load image data
            if (!LoadRawData())
                return false;

            //BGRtoRGB(); // Convert to RGB
            break;
        }

    }

    // Check flip bit
    if ((m_pData[17] & 0x20) == 0)
        FlipImg();

    // Release file memory
    m_pData = NULL;

    return true;
}

const unsigned char * TKTGAImage::GetPixel(unsigned int x, unsigned int y) const
{
    if (x < 0) x = 0;
    if (x >= m_uiWidth) x = m_uiWidth - 1;
    if (y < 0) y = 0;
    if (y >= m_uiHeight) y = m_uiHeight - 1;

    int bytes_per_pixel = m_uiBPP / 8;
    int loc = (y * m_uiWidth + x) * bytes_per_pixel;

    return m_pImageData + loc;
}


bool TKTGAImage::ReadHeader()
{
    short ColMapStart, ColMapLen;
    short x1, y1, x2, y2;

    if (m_pData == NULL)
        return false;

    if (m_pData[1] > 1)    // 0 (RGB) and 1 (Indexed) are the only types we know about
        return false;

    m_ucEnc = m_pData[2];     // Encoding flag  1 = Raw indexed image
    //                2 = Raw RGB
    //                3 = Raw greyscale
    //                9 = RLE indexed
    //               10 = RLE RGB
    //               11 = RLE greyscale
    //               32 & 33 Other compression, indexed

    if (m_ucEnc > 11)       // We don't want 32 or 33
        return false;


    // Get palette info
    TKMemcpy(&ColMapStart, &m_pData[3], 2);
    TKMemcpy(&ColMapLen, &m_pData[5], 2);

    // Reject indexed images if not a VGA palette (256 entries with 24 bits per entry)
    if (m_pData[1] == 1) // Indexed
    {
        if (ColMapStart != 0 || ColMapLen != 256 || m_pData[7] != 24)
            return false;
    }

    // Get image window and produce width & height values
    TKMemcpy(&x1, &m_pData[8], 2);
    TKMemcpy(&y1, &m_pData[10], 2);
    TKMemcpy(&x2, &m_pData[12], 2);
    TKMemcpy(&y2, &m_pData[14], 2);

    m_uiWidth = (x2 - x1);
    m_uiHeight = (y2 - y1);

    if (m_uiWidth < 1 || m_uiHeight < 1)
        return false;

    // Bits per Pixel
    m_uiBPP = m_pData[16];

    // Check flip / interleave byte
    if (m_pData[17] > 32) // Interleaved data
        return false;

    // Calculate image size
    m_uiDataSize = (m_uiWidth * m_uiHeight * (m_uiBPP / 8));

    return true;
}

bool TKTGAImage::LoadRawData()
{
    short iOffset;
    m_pImageData = TK_NEW unsigned char[m_uiDataSize];
    if (!m_pImageData)
        return false;

    iOffset = m_pData[0] + 18; // Add header to ident field size
    if (m_pData[1] == 1) // Indexed images
        iOffset += 768;  // Add palette offset

    TKMemcpy(m_pImageData, &m_pData[iOffset], m_uiDataSize);
    return true;
}

bool TKTGAImage::LoadTgaRLEData()
{
    short iOffset, iPixelSize;
    unsigned char *pCur;
    unsigned long Index = 0;
    unsigned char bLength, bLoop;

    // Calculate offset to image data
    iOffset = m_pData[0] + 18;
    // Add palette offset for indexed images
    if (m_pData[1] == 1)
        iOffset += 768;

    // Get pixel size in bytes
    iPixelSize = m_uiBPP / 8;
    // Set our pointer to the beginning of the image data
    pCur = &m_pData[iOffset];

    m_pImageData = TK_NEW unsigned char[m_uiDataSize];
    if (!m_pImageData)
        return false;

    // Decode
    while (Index < m_uiDataSize)
    {
        if (*pCur & 0x80) // Run length chunk (High bit = 1)
        {
            bLength = *pCur - 127; // Get run length
            pCur++;            // Move to pixel data

            // Repeat the next pixel bLength times
            for (bLoop = 0; bLoop != bLength; ++bLoop, Index += iPixelSize)
                TKMemcpy(&m_pImageData[Index], pCur, iPixelSize);

            pCur += iPixelSize; // Move to the next descriptor chunk
        }
        else // Raw chunk
        {
            bLength = *pCur + 1; // Get run length
            pCur++;          // Move to pixel data

            // Write the next bLength pixels directly
            for (bLoop = 0; bLoop != bLength; ++bLoop, Index += iPixelSize, pCur += iPixelSize)
                TKMemcpy(&m_pImageData[Index], pCur, iPixelSize);
        }
    }

    return true;
}

bool TKTGAImage::LoadTgaPalette()
{
    unsigned char bTemp;
    short iIndex, iPalPtr;

    // Create space for new palette
    m_pPalette = TK_NEW unsigned char[768];
    if (!m_pPalette)
        return false;

    // VGA palette is the 768 bytes following the header
    TKMemcpy(m_pPalette, &m_pData[m_pData[0] + 18], 768);

    // Palette entries are BGR ordered so we have to convert to RGB
    for (iIndex = 0, iPalPtr = 0; iIndex != 256; ++iIndex, iPalPtr += 3)
    {
        bTemp = m_pPalette[iPalPtr];               // Get Blue value
        m_pPalette[iPalPtr] = m_pPalette[iPalPtr + 2]; // Copy Red to Blue
        m_pPalette[iPalPtr + 2] = bTemp;             // Replace Blue at the end
    }

    return true;
}

void TKTGAImage::BGRtoRGB()
{
    unsigned long Index, nPixels;
    unsigned char *bCur;
    unsigned char bTemp;
    short iPixelSize;

    // Set ptr to start of image
    bCur = m_pImageData;
    // Calc number of pixels
    nPixels = m_uiWidth * m_uiHeight;
    // Get pixel size in bytes
    iPixelSize = m_uiBPP / 8;

    for (Index = 0; Index != nPixels; Index++)  // For each pixel
    {
        bTemp = *bCur;      // Get Blue value
        *bCur = *(bCur + 2);  // Swap red value into first position
        *(bCur + 2) = bTemp;  // Write back blue to last position

        bCur += iPixelSize; // Jump to next pixel
    }
}

void TKTGAImage::FlipImg()
{
    unsigned char bTemp;
    unsigned char *pLine1, *pLine2;
    int iLineLen, iIndex;

    iLineLen = m_uiWidth * (m_uiBPP / 8);
    pLine1 = m_pImageData;
    pLine2 = &m_pImageData[iLineLen * (m_uiHeight - 1)];

    for (; pLine1 < pLine2; pLine2 -= (iLineLen * 2))
    {
        for (iIndex = 0; iIndex != iLineLen; pLine1++, pLine2++, iIndex++)
        {
            bTemp = *pLine1;
            *pLine1 = *pLine2;
            *pLine2 = bTemp;
        }
    }
}
// =========================TKTGAImage End===============================

