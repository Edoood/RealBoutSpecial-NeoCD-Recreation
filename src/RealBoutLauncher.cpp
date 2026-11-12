
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commdlg.h>
#include <shlwapi.h>
#include <filesystem>
#include <string>
#include <thread>
#include <system_error>
#include <algorithm>
#include <cwctype>
#include <exception>

#pragma comment(lib, "Comdlg32.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "User32.lib")

namespace fs = std::filesystem;
static HWND windowHandle{}, romEdit{}, cdEdit{}, neoEdit{}, romButton{}, cdButton{}, neoButton{};
static HWND actionButton{}, configureButton{}, infoLabel{}, statusLabel{}, headerLabel{};
static HBITMAP artwork{};
static HFONT fontRegular{}, fontBold{}, fontTitle{};
static bool installed=false;
static bool busy=false;
static fs::path root, support, biosDir, gameExe, gameChd;
static constexpr int ID_ROM=1001, ID_CD=1002, ID_NEO=1003, ID_ACTION=1004, ID_CONFIG=1005;
static const wchar_t* ROM_NAME=L"Real Bout Fatal Fury Special.chd";
static void logLine(const std::wstring& mensagem) {
    fs::path arquivo = root / L"RealBoutLauncher.log";

    HANDLE h = CreateFileW(
        arquivo.c_str(),
        FILE_APPEND_DATA,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (h == INVALID_HANDLE_VALUE) return;

    LARGE_INTEGER tamanho{};
    DWORD escrito = 0;

    if (GetFileSizeEx(h, &tamanho) && tamanho.QuadPart == 0) {
        const wchar_t bom = 0xFEFF;
        WriteFile(h, &bom, sizeof(bom), &escrito, nullptr);
    }

    std::wstring linha = mensagem + L"\r\n";
    WriteFile(
        h,
        linha.c_str(),
        (DWORD)(linha.size() * sizeof(wchar_t)),
        &escrito,
        nullptr
    );

    CloseHandle(h);
}

static std::wstring getText(HWND h) {
    const int len=GetWindowTextLengthW(h);
    std::wstring result(static_cast<size_t>(len)+1, L'\0');
    GetWindowTextW(h,result.data(),len+1);
    result.resize(static_cast<size_t>(len));
    return result;
}
static void setText(HWND h, const std::wstring& v) {SetWindowTextW(h,v.c_str());}
static void dialogError(const std::wstring& s) {
    MessageBoxW(windowHandle,s.c_str(),L"Real Bout Online",MB_OK|MB_ICONWARNING);
}
static bool existsFile(const fs::path& p) {
    std::error_code e;
    return fs::is_regular_file(p,e);
}
static bool filesPresent() {
    return existsFile(gameChd) &&
        existsFile(biosDir/L"neocdz.zip") &&
        existsFile(biosDir/L"neogeo.zip");
}
static HWND createControl(const wchar_t* klass,const wchar_t* title,DWORD style,
                          int x,int y,int w,int h,int id=0) {
    HWND result=CreateWindowExW(0,klass,title,WS_CHILD|WS_VISIBLE|style,x,y,w,h,
                              windowHandle,reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
                              GetModuleHandleW(nullptr),nullptr);
    SendMessageW(result,WM_SETFONT,reinterpret_cast<WPARAM>(fontRegular),TRUE);
    return result;
}
static void setMode() {
    installed=filesPresent();
    for(HWND c:{romEdit,cdEdit,neoEdit,romButton,cdButton,neoButton}) ShowWindow(c,installed?SW_HIDE:SW_SHOW);
    ShowWindow(configureButton,installed?SW_SHOW:SW_HIDE);
    setText(headerLabel,installed?L"PRONTO PARA JOGAR":L"CONFIGURAR JOGO");
    setText(actionButton,installed?L"PLAY":L"INSTALAR E JOGAR");
    setText(infoLabel,installed?
        L"F1  -  Abre as opcoes do jogo\r\nSTEAM  -  Obrigatoria para jogar online\r\nNETPLAY  -  Conexao pelo sistema existente":
        L"Selecione seu jogo (.CHD) e as duas BIOS.\r\nOs arquivos serao copiados para a pasta Support.\r\nNao inclui ROM nem BIOS no download.");
    setText(statusLabel,installed?L"Arquivos encontrados. Pronto para iniciar.":L"Aguardando selecao de arquivos.");
}
static void pick(HWND edit,const wchar_t* filter) {
    wchar_t name[32768]={};
    OPENFILENAMEW ofn{};
    ofn.lStructSize=sizeof(ofn);
    ofn.hwndOwner=windowHandle;
    ofn.lpstrFile=name;
    ofn.nMaxFile=32768;
    ofn.lpstrFilter=filter;
    ofn.nFilterIndex=1;
    ofn.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR;
    if(GetOpenFileNameW(&ofn)) setText(edit,name);
}
static void launchGame() {
    if(!existsFile(gameExe)) {
        dialogError(L"Jogar Real Bout Online.exe nao foi encontrado na pasta Support.");
        return;
    }
    std::wstring command=L"\""+gameExe.wstring()+L"\"";
    STARTUPINFOW si{}; si.cb=sizeof(si);
    PROCESS_INFORMATION pi{};
    // Working directory is crucial: the original executable expects the CHD beside itself.
    BOOL ok=CreateProcessW(gameExe.c_str(),command.data(),nullptr,nullptr,FALSE,
                           0,nullptr,support.c_str(),&si,&pi);
    if(ok) {
        CloseHandle(pi.hThread); CloseHandle(pi.hProcess);
        setText(statusLabel,L"Jogo iniciado. Steam precisa estar aberta para jogar online.");
    } else dialogError(L"Falha ao iniciar o jogo. Codigo Windows: "+std::to_wstring(GetLastError()));
}
static void performAction() {
    if(busy) return;
    if(installed) {launchGame();return;}
    fs::path rom=getText(romEdit), cd=getText(cdEdit), neo=getText(neoEdit);
    logLine(L"==================================");
    logLine(L"INICIO DA VERIFICACAO");
    logLine(L"ROM: " + rom.wstring());
    logLine(L"BIOS CD: " + cd.wstring());
    logLine(L"BIOS NEO: " + neo.wstring());

    logLine(L"ROM EXISTE: " + std::wstring(existsFile(rom) ? L"SIM" : L"NAO"));
    logLine(L"BIOS CD EXISTE: " + std::wstring(existsFile(cd) ? L"SIM" : L"NAO"));
    logLine(L"BIOS NEO EXISTE: " + std::wstring(existsFile(neo) ? L"SIM" : L"NAO"));

    logLine(L"EXTENSAO ROM: " + rom.extension().wstring());
    logLine(L"NOME BIOS CD: " + cd.filename().wstring());
    logLine(L"NOME BIOS NEO: " + neo.filename().wstring());
    if(!existsFile(rom)||!existsFile(cd)||!existsFile(neo)) {
        dialogError(L"Selecione os tres arquivos: CHD, neocdz.zip e neogeo.zip.");
        return;
    }
    auto lowercase=[](std::wstring value) {
        std::transform(value.begin(),value.end(),value.begin(),towlower);return value;
    };
    if(lowercase(rom.extension().wstring())!=L".chd" ||
       lowercase(cd.filename().wstring())!=L"neocdz.zip" ||
       lowercase(neo.filename().wstring())!=L"neogeo.zip") {
        dialogError(L"Verifique os nomes: jogo .chd, neocdz.zip e neogeo.zip.");
        return;
    }
    if(!existsFile(gameExe)) {dialogError(L"Executavel original nao encontrado em Support.");return;}
    busy=true;EnableWindow(actionButton,FALSE);
    setText(statusLabel,L"Copiando os arquivos. Aguarde...");
    try {
        std::error_code ec;
        fs::create_directories(biosDir);
        auto copyUnlessSame=[](const fs::path& from,const fs::path& to) {
            std::error_code compareError;
            const bool same=fs::equivalent(from,to,compareError);
            if(!compareError && same) return;
            fs::copy_file(from,to,fs::copy_options::overwrite_existing);
        };
        copyUnlessSame(rom,gameChd);
        copyUnlessSame(cd,biosDir/L"neocdz.zip");
        copyUnlessSame(neo,biosDir/L"neogeo.zip");
        setMode();
        launchGame();
    } catch(const std::exception&) {
        dialogError(L"Nao foi possivel copiar os arquivos. Verifique espaco livre e permissoes.");
    }
    busy=false;EnableWindow(actionButton,TRUE);
}
static LRESULT CALLBACK mainProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp) {
    switch(msg) {
    case WM_CREATE: {
        windowHandle=hwnd;
        fontRegular=CreateFontW(21,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
                   OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");
        fontBold=CreateFontW(21,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
                   OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");
        fontTitle=CreateFontW(33,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
                   OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");
        headerLabel=createControl(L"STATIC",L"CONFIGURAR JOGO",SS_LEFT,28,250,825,45);
        SendMessageW(headerLabel,WM_SETFONT,reinterpret_cast<WPARAM>(fontTitle),TRUE);
        createControl(L"STATIC",L"ROM / CHD:",0,28,304,200,28);
        romEdit=createControl(L"EDIT",L"",WS_BORDER|ES_AUTOHSCROLL|ES_READONLY,28,333,650,32);
        romButton=createControl(L"BUTTON",L"PROCURAR ROM",BS_PUSHBUTTON,694,329,180,40,ID_ROM);
        createControl(L"STATIC",L"BIOS NEO GEO CD (neocdz.zip):",0,28,381,600,28);
        cdEdit=createControl(L"EDIT",L"",WS_BORDER|ES_AUTOHSCROLL|ES_READONLY,28,410,650,32);
        cdButton=createControl(L"BUTTON",L"PROCURAR BIOS CD",BS_PUSHBUTTON,694,406,180,40,ID_CD);
        createControl(L"STATIC",L"BIOS NEO GEO (neogeo.zip):",0,28,458,600,28);
        neoEdit=createControl(L"EDIT",L"",WS_BORDER|ES_AUTOHSCROLL|ES_READONLY,28,487,650,32);
        neoButton=createControl(L"BUTTON",L"PROCURAR BIOS NEO",BS_PUSHBUTTON,694,483,180,40,ID_NEO);
        infoLabel=createControl(L"STATIC",L"",SS_LEFT,28,533,800,85);
        actionButton=createControl(L"BUTTON",L"INSTALAR E JOGAR",BS_DEFPUSHBUTTON,625,622,249,50,ID_ACTION);
        SendMessageW(actionButton,WM_SETFONT,reinterpret_cast<WPARAM>(fontBold),TRUE);
        configureButton=createControl(L"BUTTON",L"RECONFIGURAR",BS_PUSHBUTTON,28,622,175,48,ID_CONFIG);
        statusLabel=createControl(L"STATIC",L"",SS_LEFT,220,688,654,30);
        fs::path banner=support/L"banner.bmp";
        artwork=static_cast<HBITMAP>(LoadImageW(nullptr,banner.c_str(),IMAGE_BITMAP,0,0,LR_LOADFROMFILE));
        setMode();
        return 0;
    }
    case WM_COMMAND:
        switch(LOWORD(wp)) {
        case ID_ROM: pick(romEdit,L"Imagem CHD (*.chd)\0*.chd\0\0"); return 0;
        case ID_CD: pick(cdEdit,L"Arquivo ZIP (*.zip)\0*.zip\0\0"); return 0;
        case ID_NEO: pick(neoEdit,L"Arquivo ZIP (*.zip)\0*.zip\0\0"); return 0;
        case ID_ACTION: performAction();return 0;
        case ID_CONFIG:
            for(HWND c:{romEdit,cdEdit,neoEdit,romButton,cdButton,neoButton}) ShowWindow(c,SW_SHOW);
            ShowWindow(configureButton,SW_HIDE);
            installed=false;
            setText(headerLabel,L"RECONFIGURAR ARQUIVOS");
            setText(actionButton,L"INSTALAR E JOGAR");
            setText(infoLabel,L"Selecione novos arquivos. Os anteriores serao substituidos.");
            return 0;
        }
        break;
    case WM_PAINT: {
        PAINTSTRUCT ps; HDC dc=BeginPaint(hwnd,&ps);
        RECT r;GetClientRect(hwnd,&r);
        HBRUSH dark=CreateSolidBrush(RGB(23,24,33));
        FillRect(dc,&r,dark);DeleteObject(dark);
        if(artwork){
            BITMAP bmp{};GetObjectW(artwork,sizeof(bmp),&bmp);
            HDC mem=CreateCompatibleDC(dc);
            HGDIOBJ old=SelectObject(mem,artwork);
            SetStretchBltMode(dc,HALFTONE);
            StretchBlt(dc,0,0,902,230,mem,0,0,bmp.bmWidth,bmp.bmHeight,SRCCOPY);
            SelectObject(mem,old);DeleteDC(mem);
        }
        EndPaint(hwnd,&ps);return 0;
    }
    case WM_CTLCOLORSTATIC: {
        HDC dc=reinterpret_cast<HDC>(wp);
        SetTextColor(dc,RGB(240,240,240));
        SetBkColor(dc,RGB(23,24,33));
        static HBRUSH textBackground=CreateSolidBrush(RGB(23,24,33));
        return reinterpret_cast<LRESULT>(textBackground);
    }
    case WM_DESTROY:
        if(artwork)DeleteObject(artwork);
        DeleteObject(fontRegular);DeleteObject(fontBold);DeleteObject(fontTitle);
        PostQuitMessage(0);return 0;
    }
    return DefWindowProcW(hwnd,msg,wp,lp);
}
int WINAPI wWinMain(HINSTANCE instance,HINSTANCE,LPWSTR,int show) {
    wchar_t filename[32768]{};
    if(!GetModuleFileNameW(nullptr,filename,32768))return 1;
    root=fs::path(filename).parent_path();
    support=root/L"Support";
    biosDir=support/L"rffs-runtime"/L"system"/L"fbneo";
    gameExe=support/L"Jogar Real Bout Online.exe";
    gameChd=support/ROM_NAME;
    WNDCLASSEXW wc{};wc.cbSize=sizeof(wc);
    wc.lpfnWndProc=mainProc;
    wc.hInstance=instance;wc.lpszClassName=L"RealBoutOnlineWin32Launcher";
    wc.hCursor=LoadCursorW(nullptr,IDC_ARROW);
    wc.hbrBackground=CreateSolidBrush(RGB(23,24,33));
    if(!RegisterClassExW(&wc))return 1;
    HWND hwnd=CreateWindowExW(0,wc.lpszClassName,L"REAL BOUT SPECIAL | ONLINE LAUNCHER",
        WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,
        CW_USEDEFAULT,CW_USEDEFAULT,918,775,nullptr,nullptr,instance,nullptr);
    if(!hwnd)return 1;
    ShowWindow(hwnd,show);UpdateWindow(hwnd);
    MSG msg{};
    while(GetMessageW(&msg,nullptr,0,0)>0) {
        TranslateMessage(&msg);DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}
