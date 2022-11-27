/*****************************************************************************************
 File:     CBPropertySheet.h
 Author:   Ovidiu Cucu - Microsoft MVP Viusal C++
                         Codeguru nickname: ovidiucucu
                         Homepage: www.codexpert.ro
 Updated:  January 02, 2010
 Contents: CCBPropertySheet class definition.
 Remarks:  CCBPropertySheet is an MFC-extension class for creating properties sheets
           which are using a custom font.
******************************************************************************************/
#pragma once

#define DOMODAL_RET INT_PTR

class CCBPropertySheet : public CPropertySheet
{
    DECLARE_DYNAMIC(CCBPropertySheet)
    public:
    CCBPropertySheet(UINT nIDCaption, CWnd *pParentWnd = NULL, UINT iSelectPage = 0);
    CCBPropertySheet(LPCTSTR pszCaption, CWnd *pParentWnd = NULL, UINT iSelectPage = 0);
    virtual ~CCBPropertySheet() = 0;

    // Attributes
    private:
    static __declspec(thread) WORD m_wFontSize;
    static __declspec(thread) LPCTSTR m_pszFontFaceName;

    // Operations
    public:
    // call this function to create a modal property sheet with custom font.
    DOMODAL_RET DoModal(LPCTSTR pszFontFaceName, WORD wFontSize);

    // call this function to create a modal property sheet with default font.
    virtual DOMODAL_RET DoModal();

    // call this function to create a modeless property sheet with custom font.
    BOOL Create(LPCTSTR pszFontFaceName, WORD wFontSize, CWnd *pParentWnd = NULL,
                DWORD dwStyle = (DWORD)-1, DWORD dwExStyle = 0);

    // call this function to create a modeless property sheet with default font.
    BOOL Create(CWnd *pParentWnd = NULL, DWORD dwStyle = (DWORD)-1, DWORD dwExStyle = 0);

    // Overides
    protected:
    virtual void BuildPropPageArray();

    // Implementation
    private:
    static int CALLBACK PropSheetProc(HWND hWndDlg, UINT uMsg, LPARAM lParam);
    void                Init(LPCTSTR pszFontFaceName, WORD wFontSize);
};
