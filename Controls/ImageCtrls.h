#if !defined(AFX_IMAGE_CONTROLS_H__558E5844_389D_11D4_8A51_83C94F0AD91B__INCLUDED_)
#define AFX_IMAGE_CONTROLS_H__558E5844_389D_11D4_8A51_83C94F0AD91B__INCLUDED_

#include "stdafx.h"
#include "Appdef.h"
//C:\Ilya\Programming\cpp\Numbering\Controls\ImageCtrls.h
#include "MGTools\Include\Utils\BaseDataType.h"
#include "MGTools\Include\Image\SecDib.h"
#include "ApertureCore\include\aperturecore\visibility\IDataProviders.h"  // ← Add interface

class CImageCtrls : public IImageData  // ← Implement interface
{
public:	
    double kContrast;
    double kBright;

    double undokContrast;
    double undokBright;

	CString OriginalPath;
    CSize ImageSize;
    SECDib* m_pDIB;
    CString FileImagePic;
    RGBQUAD m_rgbPalette[256];

public:	
    CImageCtrls();
	~CImageCtrls();
	void Init();
    void PrepareNewImage();

    SECImage* GetImage() {return (SECImage *) m_pDIB; }
    CRect GetDIBRect();
    void SaveImage(LPCTSTR fname);
    bool LoadImage(CString& fname);
    BOOL ConvertToDIB(CString& name);
	BOOL ConvertToGrayScale(CDC* pDC, LPCTSTR file);
    void OnFileSaveAsDib(LPCTSTR fname);
    void OnFileSaveAsGif(LPCTSTR fname);
    void OnFileSaveAsJpeg(LPCTSTR fname);
    void OnFileSaveAsPcx(LPCTSTR fname);
    void OnFileSaveAsTga(LPCTSTR fname);
    void OnFileSaveAsTiff(LPCTSTR fname);

    CImageCtrls(const CImageCtrls& rhs){
      { operator=(rhs);}
     }

    CImageCtrls& operator=(const CImageCtrls& rhs);
    
    // IImageData interface implementation
    bool HasImage() const override { return m_pDIB != nullptr; }
    int GetWidth() const override { return ImageSize.cx; }
    int GetHeight() const override { return ImageSize.cy; }
    const unsigned char* GetBitmapData() const override;
    unsigned char GetPixel(int x, int y) const override;
    uint64_t GetImageVersion() const override { return imageVersion_; }

	// Invalidate the image (e.g., after loading a new image or modifying it)
	void InvalidateImage() { ++imageVersion_; }

private:
	uint64_t imageVersion_ = 0;  // Increment on image change
};

#endif // !defined(AFX_IMAGE_CONTROLS_DEFS_H__558E5844_389D_11D4_8A51_83C94F0AD91B__INCLUDED_)

