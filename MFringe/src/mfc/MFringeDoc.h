#pragma once

class CMFringeDoc : public CDocument
{
protected:
	CMFringeDoc();
	DECLARE_DYNCREATE(CMFringeDoc)

public:
	virtual ~CMFringeDoc();

	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	DECLARE_MESSAGE_MAP()
};