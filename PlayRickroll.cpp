/*The Code is made by AI
Fucccccccccccccccccccccccccck yoooooooooou ai*/
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <evr.h>
#include <shlwapi.h>
#include <cstdint>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "evr.lib")
#pragma comment(lib, "shlwapi.lib")

// MinGW 头文件可能缺少此声明
extern "C" HRESULT WINAPI MFCreateMFByteStreamOnStream(
    IStream* pStream,
    IMFByteStream** ppByteStream
);

// objcopy 生成的符号
extern "C" {
    extern const uint8_t _binary_rickroll_mp4_start[];
    extern const uint8_t _binary_rickroll_mp4_end[];
}

struct MemBuffer {
    const uint8_t* data;
    size_t size;
};

static bool LoadRes(MemBuffer& out) {
    out.data = _binary_rickroll_mp4_start;
    out.size = (size_t)(_binary_rickroll_mp4_end - _binary_rickroll_mp4_start);
    return out.size > 0;
}

HWND g_hwnd = NULL;

LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    if (m == WM_DESTROY) { PostQuitMessage(0); return 0; }
    return DefWindowProcA(h, m, w, l);
}

HWND MakeWindow() {
    WNDCLASSA wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "RR";
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    RegisterClassA(&wc);

    RECT rc = { 0, 0, 1280, 720 };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hwnd = CreateWindowExA(
        0, "RR", "Rickroll!!!!!!!!",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right - rc.left, rc.bottom - rc.top,
        NULL, NULL, GetModuleHandle(NULL), NULL);

    if (hwnd) {
        SetWindowTextA(hwnd, "Rickroll!!!!!!!!");
    }
    return hwnd;
}

class SessionCallback : public IMFAsyncCallback {
    LONG m_ref = 1;
    IMFMediaSession* m_session;
    HANDLE m_done;
public:
    SessionCallback(IMFMediaSession* s, HANDLE done) : m_session(s), m_done(done) {
        m_session->AddRef();
    }
    ~SessionCallback() { m_session->Release(); }

    STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override {
        if (riid == IID_IUnknown || riid == IID_IMFAsyncCallback) {
            *ppv = static_cast<IMFAsyncCallback*>(this);
            AddRef();
            return S_OK;
        }
        return E_NOINTERFACE;
    }
    STDMETHODIMP_(ULONG) AddRef() override { return InterlockedIncrement(&m_ref); }
    STDMETHODIMP_(ULONG) Release() override {
        LONG r = InterlockedDecrement(&m_ref);
        if (r == 0) delete this;
        return r;
    }
    STDMETHODIMP GetParameters(DWORD* flags, DWORD* queue) override {
        *flags = 0;
        *queue = MFASYNC_CALLBACK_QUEUE_STANDARD;
        return S_OK;
    }
    STDMETHODIMP Invoke(IMFAsyncResult* result) override {
        MediaEventType met = MEUnknown;
        IMFMediaEvent* ev = nullptr;
        HRESULT hr = m_session->EndGetEvent(result, &ev);
        if (SUCCEEDED(hr) && ev) {
            ev->GetType(&met);
            ev->Release();
        }
        if (met == MESessionClosed || met == MEEndOfPresentation) {
            SetEvent(m_done);
        } else {
            m_session->BeginGetEvent(this, nullptr);
        }
        return S_OK;
    }
};

static bool BuildTopology(IMFMediaSource* source, IMFTopology* topology) {
    IMFPresentationDescriptor* presDesc = nullptr;
    if (FAILED(source->CreatePresentationDescriptor(&presDesc))) return false;

    DWORD streamCount = 0;
    presDesc->GetStreamDescriptorCount(&streamCount);

    for (DWORD i = 0; i < streamCount; i++) {
        BOOL selected = FALSE;
        IMFStreamDescriptor* streamDesc = nullptr;
        if (FAILED(presDesc->GetStreamDescriptorByIndex(i, &selected, &streamDesc))) continue;
        if (!selected) {
            streamDesc->Release();
            continue;
        }

        IMFMediaTypeHandler* handler = nullptr;
        streamDesc->GetMediaTypeHandler(&handler);
        GUID majorType = GUID_NULL;
        handler->GetMajorType(&majorType);

        IMFTopologyNode* srcNode = nullptr;
        MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &srcNode);
        srcNode->SetUnknown(MF_TOPONODE_SOURCE, source);
        srcNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, presDesc);
        srcNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, streamDesc);
        topology->AddNode(srcNode);

        IMFTopologyNode* outNode = nullptr;
        MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &outNode);

        IMFActivate* activate = nullptr;
        if (majorType == MFMediaType_Video) {
            MFCreateVideoRendererActivate(g_hwnd, &activate);
        } else if (majorType == MFMediaType_Audio) {
            MFCreateAudioRendererActivate(&activate);
        }

        if (activate) {
            outNode->SetObject(activate);
            activate->Release();
            topology->AddNode(outNode);
            srcNode->ConnectOutput(0, outNode, 0);
        }

        outNode->Release();
        srcNode->Release();
        handler->Release();
        streamDesc->Release();
    }

    presDesc->Release();
    return true;
}

void PlayRickroll() {
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);

    IMFSourceResolver* resolver = nullptr;
    IUnknown* sourceObj = nullptr;
    IMFMediaSource* source = nullptr;
    IMFMediaSession* session = nullptr;
    IMFTopology* topology = nullptr;
    SessionCallback* callback = nullptr;
    HANDLE done = NULL;
    IStream* pStream = nullptr;
    IMFByteStream* pByteStream = nullptr;
    IMFAttributes* pAttrs = nullptr;

    MemBuffer buf;
    if (!LoadRes(buf)) {
        MessageBoxA(NULL, "资源加载失败", "Error", MB_OK | MB_ICONERROR);
        goto finish;
    }

    // 1. 从内存创建 IStream
    pStream = SHCreateMemStream(buf.data, (UINT)buf.size);
    if (!pStream) {
        MessageBoxA(NULL, "创建内存流失败", "Error", MB_OK | MB_ICONERROR);
        goto finish;
    }

    // 2. 包装为 IMFByteStream
    if (FAILED(MFCreateMFByteStreamOnStream(pStream, &pByteStream))) {
        MessageBoxA(NULL, "创建字节流失败", "Error", MB_OK | MB_ICONERROR);
        goto finish;
    }

    // 3. 给字节流设置 MIME 类型，让 MF 知道这是 MP4
    if (SUCCEEDED(pByteStream->QueryInterface(IID_PPV_ARGS(&pAttrs)))) {
        pAttrs->SetString(MF_BYTESTREAM_CONTENT_TYPE, L"video/mp4");
        pAttrs->Release();
        pAttrs = nullptr;
    }

    // 4. 用 SourceResolver 从字节流创建媒体源
    if (FAILED(MFCreateSourceResolver(&resolver))) {
        MessageBoxA(NULL, "创建 SourceResolver 失败", "Error", MB_OK | MB_ICONERROR);
        goto finish;
    }

    {
        MF_OBJECT_TYPE objType = MF_OBJECT_INVALID;

        HRESULT hr = resolver->CreateObjectFromByteStream(
            pByteStream,
            L"file.mp4",
            MF_RESOLUTION_MEDIASOURCE,
            NULL,
            &objType,
            &sourceObj);

        if (FAILED(hr)) {
            char msg[128];
            wsprintfA(msg, "无法解析媒体源 (HRESULT: 0x%08X)", hr);
            MessageBoxA(NULL, msg, "Error", MB_OK | MB_ICONERROR);
            goto finish;
        }
    }

    if (FAILED(sourceObj->QueryInterface(IID_PPV_ARGS(&source)))) {
        MessageBoxA(NULL, "无法获取媒体源接口", "Error", MB_OK | MB_ICONERROR);
        goto finish;
    }

    // 5. 创建窗口
    g_hwnd = MakeWindow();
    if (!g_hwnd) {
        MessageBoxA(NULL, "窗口创建失败", "Error", MB_OK | MB_ICONERROR);
        goto finish;
    }

    // 6. 创建会话
    if (FAILED(MFCreateMediaSession(NULL, &session))) {
        MessageBoxA(NULL, "创建会话失败", "Error", MB_OK | MB_ICONERROR);
        goto finish;
    }

    // 7. 创建拓扑
    if (FAILED(MFCreateTopology(&topology))) {
        MessageBoxA(NULL, "创建拓扑失败", "Error", MB_OK | MB_ICONERROR);
        goto finish;
    }
    if (!BuildTopology(source, topology)) {
        MessageBoxA(NULL, "构建拓扑失败", "Error", MB_OK | MB_ICONERROR);
        goto finish;
    }

    // 8. 事件回调
    done = CreateEventW(NULL, FALSE, FALSE, NULL);
    callback = new SessionCallback(session, done);
    session->BeginGetEvent(callback, nullptr);

    if (FAILED(session->SetTopology(0, topology))) {
        MessageBoxA(NULL, "设置拓扑失败", "Error", MB_OK | MB_ICONERROR);
        goto finish;
    }

    {
        PROPVARIANT varStart;
        PropVariantInit(&varStart);
        session->Start(&GUID_NULL, &varStart);
    }

    // 9. 消息循环 + 等待播放结束
    {
        bool quit = false;
        while (!quit) {
            DWORD waitResult = MsgWaitForMultipleObjects(1, &done, FALSE, 100, QS_ALLINPUT);
            if (waitResult == WAIT_OBJECT_0) {
                quit = true;
            } else if (waitResult == WAIT_OBJECT_0 + 1) {
                MSG msg;
                while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                    if (msg.message == WM_QUIT) { quit = true; break; }
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
            if (!IsWindow(g_hwnd)) quit = true;
        }
    }

    session->Close();
    session->Shutdown();

finish:
    if (pAttrs) pAttrs->Release();
    if (pByteStream) pByteStream->Release();
    if (pStream) pStream->Release();
    if (callback) callback->Release();
    if (done) CloseHandle(done);
    if (topology) topology->Release();
    if (session) session->Release();
    if (source) source->Release();
    if (sourceObj) sourceObj->Release();
    if (resolver) resolver->Release();
    if (g_hwnd) { DestroyWindow(g_hwnd); g_hwnd = NULL; }
    MFShutdown();
    CoUninitialize();
}