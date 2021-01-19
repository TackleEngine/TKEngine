#ifndef TK_IMAGE_H
#define TK_IMAGE_H
#include "TKSystem.h"
#include "TKMemManager.h"
#include "TKFile.h"
TK_NAMESPACE_BEGIN

    class TKSYSTEM_API TKImage
    {
    public:
        enum	// Image Format
        {
            IF_BMP,		// 不支持压缩
            IF_TGA,		// 支持压缩
            IF_MAX
        };

        static TCHAR ms_ImageFormat[IF_MAX][10];
        TKImage();
        virtual ~TKImage();

        virtual bool Load(const TCHAR* pFileName) = 0;
        virtual bool LoadFormBuffer(unsigned char* pBuffer, unsigned int uiSize) = 0;
        virtual const unsigned char* GetPixel(unsigned int x, unsigned int y) const = 0;	//获取指向对应位置的像素指针

        FORCEINLINE unsigned int GetWidth() const
        {
            return m_uiWidth;
        }
        FORCEINLINE	unsigned int GetHight() const
        {
            return m_uiHeight;
        }
        FORCEINLINE unsigned int GetBPP() const
        {
            return m_uiBPP;
        }
        FORCEINLINE const unsigned char* GetImgData() const
        {
            return m_pImageData;
        }
        FORCEINLINE unsigned int GetImgDataSize() const
        {
            return m_uiDataSize;
        }
        FORCEINLINE unsigned char* GetPalette() const	// Return a pointer to VGA palette
        {
            return m_pPalette;
        }
    protected:
        unsigned int m_uiWidth;
        unsigned int m_uiHeight;
        unsigned int m_uiBPP;			// 像素深度
        unsigned char* m_pImageData;
        unsigned int m_uiDataSize;
        unsigned char* m_pData;
        unsigned char* m_pPalette;		// a pointer to VGA palette(调色板)
    };

// 支持BMP格式的图片读取类
    class TKSYSTEM_API TKBMPImage : public TKImage
    {
    public:
        TKBMPImage();
        virtual ~TKBMPImage();

        virtual bool Load(const TCHAR* pFileName);
        virtual bool LoadFormBuffer(unsigned char* pBuffer, unsigned int uiSize);
        virtual const unsigned char* GetPixel(unsigned int x, unsigned int y) const;

    private:
        // Internal workers
        bool GetFile(const TCHAR* pFilename);
        bool ReadBmpHeader();
        bool LoadBmpRaw();
        bool LoadBmpRLE8();
        bool LoadBmpPalette();
        void FlipImg(); // Inverts image data, BMP is stored in reverse scanline order

    private:
        unsigned int m_uiEnc;
        unsigned int m_uiPlanes;
        unsigned int m_uiImgOffset;
    };

// 支持TGA格式的图片读取类
    class TKSYSTEM_API TKTGAImage : public TKImage
    {
    public:
        TKTGAImage();
        virtual ~TKTGAImage();

        virtual bool Load(const TCHAR* pFileName);
        virtual bool LoadFormBuffer(unsigned char* pBuffer, unsigned int uiSize);
        virtual const unsigned char* GetPixel(unsigned int x, unsigned int y) const;

    private:
        // Internal workers
        bool ReadHeader();
        bool LoadRawData();
        bool LoadTgaRLEData();
        bool LoadTgaPalette();
        void BGRtoRGB();
        void FlipImg();

    private:
        unsigned char m_ucEnc;
    };

TK_NAMESPACE_END

#endif

