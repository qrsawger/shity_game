#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <evr.h>
/*The Code is made by AI
Fucccccccccccccccccccccccccck yoooooooooou ai*/
#include <shlwapi.h>
#include <string>
#include <cstdint>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "evr.lib")
#pragma comment(lib, "shlwapi.lib")

#define IDR_VIDEO 101

struct MemBuffer {
    uint8_t* data;
    size_t size;
};

static bool LoadRes(int id, MemBuffer& out) {
    HRSRC h = FindResourceW(NULL, MAKEINTRESOURCEW(id), MAKEINTRESOURCEW(10));
    if (!h) return false;
    HGLOBAL m = LoadResource(NULL, h);
    if (!m) return false;
    out.data = (uint8_t*)LockResource(m);
    out.size = SizeofResource(NULL, h);
    return out.data && out.size > 0;
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

void PlayRickroll() {
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);

    MemBuffer buf;
    if (!LoadRes(IDR_VIDEO, buf)) {
        MFShutdown();
        CoUninitialize();
        return;
    }

    // 写入临时文件
    wchar_t tempPath[MAX_PATH];
    wchar_t tempFile[MAX_PATH];
    GetTempPathW(MAX_PATH, tempPath);
    GetTempFileNameW(tempPath, L"RR", 0, tempFile);

    HANDLE hFile = CreateFileW(tempFile, GENERIC_WRITE, 0, NULL,
                               CREATE_ALWAYS, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        MFShutdown();
        CoUninitialize();
        return;
    }
    DWORD written;
    WriteFile(hFile, buf.data, (DWORD)buf.size, &written, NULL);
    CloseHandle(hFile);

    // 创建媒体源
    IMFSourceResolver* resolver = nullptr;
    IUnknown* sourceObj = nullptr;
    IMFMediaSource* source = nullptr;
    MFCreateSourceResolver(&resolver);
    resolver->CreateObjectFromURL(tempFile, MF_RESOLUTION_MEDIASOURCE,
                                  NULL, NULL, &sourceObj);
    sourceObj->QueryInterface(IID_PPV_ARGS(&source));

    // 创建媒体会话
    IMFMediaSession* session = nullptr;
    MFCreateMediaSession(NULL, &session);

    // 创建播放拓扑
    IMFTopology* topology = nullptr;
    MFCreateTopology(&topology);

    IMFPresentationDescriptor* presDesc = nullptr;
    source->CreatePresentationDescriptor(&presDesc);
    DWORD streamCount = 0;
    presDesc->GetStreamDescriptorCount(&streamCount);

    for (DWORD i = 0; i < streamCount; i++) {
        BOOL selected = FALSE;
        IMFStreamDescriptor* streamDesc = nullptr;
        presDesc->GetStreamDescriptorByIndex(i, &selected, &streamDesc);
        if (!selected) {
            streamDesc->Release();
            continue;
        }

        IMFMediaTypeHandler* handler = nullptr;
        streamDesc->GetMediaTypeHandler(&handler);
        GUID majorType;
        handler->GetMajorType(&majorType);

        // 源节点
        IMFTopologyNode* srcNode = nullptr;
        MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &srcNode);
        srcNode->SetUnknown(MF_TOPONODE_SOURCE, source);
        srcNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, presDesc);
        srcNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, streamDesc);
        topology->AddNode(srcNode);

        // 输出节点
        IMFTopologyNode* outNode = nullptr;
        MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &outNode);

        IMFActivate* activate = nullptr;
        if (majorType == MFMediaType_Video) {
            MFCreateVideoRendererActivate(NULL, &activate);
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

    // 开始播放
    HANDLE done = CreateEventW(NULL, FALSE, FALSE, NULL);
    SessionCallback* callback = new SessionCallback(session, done);
    session->BeginGetEvent(callback, nullptr);

    session->SetTopology(0, topology);

    PROPVARIANT varStart;
    PropVariantInit(&varStart);
    session->Start(&GUID_NULL, &varStart);

    // 等待播放结束
    WaitForSingleObject(done, INFINITE);

    // 清理
    session->Close();
    session->Shutdown();

    callback->Release();
    CloseHandle(done);

    presDesc->Release();
    topology->Release();
    session->Release();
    source->Release();
    sourceObj->Release();
    resolver->Release();

    DeleteFileW(tempFile);

    MFShutdown();
    CoUninitialize();
}