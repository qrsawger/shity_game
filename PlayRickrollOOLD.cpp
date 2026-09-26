/*The Code is made by AI
Fucccccccccccccccccccccccccck yoooooooooou ai*/
#include <windows.h>
#include <mmsystem.h>
#include <vector>
#include <cstring>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}
#include <FLAC++/decoder.h>

#pragma comment(lib, "winmm.lib")

#define IDR_VIDEO 101
#define IDR_AUDIO 102

struct MemBuffer { uint8_t* data; size_t size; size_t pos; };

bool LoadRes(int id, MemBuffer& out) {
	HRSRC h = FindResourceW(NULL, MAKEINTRESOURCEW(id), RT_RCDATA);
	if (!h) return false;
	HGLOBAL m = LoadResource(NULL, h);
	if (!m) return false;
	out.data = (uint8_t*)LockResource(m);
	out.size = SizeofResource(NULL, h);
	out.pos = 0;
	return out.data && out.size > 0;
}

static int ffmpeg_read(void* opaque, uint8_t* buf, int buf_size) {
	MemBuffer* mb = (MemBuffer*)opaque;
	size_t rem = mb->size - mb->pos;
	int n = (buf_size < (int)rem) ? buf_size : (int)rem;
	if (n <= 0) return AVERROR_EOF;
	memcpy(buf, mb->data + mb->pos, n);
	mb->pos += n;
	return n;
}

class MemFLAC : public FLAC::Decoder::Stream {
public:
	MemFLAC(MemBuffer* mb) : mb(mb) {}
	std::vector<int16_t> pcm;
	int rate = 0, ch = 0, bits = 0;
protected:
	::FLAC__StreamDecoderReadStatus read_callback(FLAC__byte b[], size_t* n) override {
		size_t rem = mb->size - mb->pos;
		if (*n > rem) *n = rem;
		if (*n == 0) return ::FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
		memcpy(b, mb->data + mb->pos, *n);
		mb->pos += *n;
		return ::FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
	}
	::FLAC__StreamDecoderWriteStatus write_callback(
		const ::FLAC__Frame* f, const FLAC__int32* const buf[]) override {
		if (!rate) { rate = f->header.sample_rate; ch = f->header.channels; bits = f->header.bits_per_sample; }
		for (size_t i = 0; i < f->header.blocksize; i++)
			for (int c = 0; c < ch; c++)
				pcm.push_back((int16_t)buf[c][i]);
		return ::FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
	}
	void error_callback(::FLAC__StreamDecoderErrorStatus) override {}
private:
	MemBuffer* mb;
};

HWAVEOUT g_wo = NULL;
WAVEHDR g_wh = {};

void PlayAudio(const std::vector<int16_t>& pcm, int rate, int ch, int bits) {
	if (pcm.empty()) return;
	WAVEFORMATEX wf = {};
	wf.wFormatTag = WAVE_FORMAT_PCM;
	wf.nChannels = ch;
	wf.nSamplesPerSec = rate;
	wf.wBitsPerSample = bits;
	wf.nBlockAlign = ch * bits / 8;
	wf.nAvgBytesPerSec = rate * wf.nBlockAlign;
	if (waveOutOpen(&g_wo, WAVE_MAPPER, &wf, 0, 0, CALLBACK_NULL)) return;
	g_wh.lpData = (LPSTR)pcm.data();
	g_wh.dwBufferLength = (DWORD)(pcm.size() * sizeof(int16_t));
	waveOutPrepareHeader(g_wo, &g_wh, sizeof(WAVEHDR));
	waveOutWrite(g_wo, &g_wh, sizeof(WAVEHDR));
}

HWND g_hwnd = NULL;
int g_w = 0, g_h = 0;

LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l) {
	if (m == WM_DESTROY) { PostQuitMessage(0); return 0; }
	return DefWindowProc(h, m, w, l);
}

HWND MakeWindow() {
	WNDCLASSW wc = {};
	wc.lpfnWndProc = WndProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = L"RR";
	RegisterClassW(&wc);

	return CreateWindowExW(
		0, L"RR", L"Rickroll!!!!!!!!",	/*It's me*/
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT,
		1280, 720,
		NULL, NULL, GetModuleHandle(NULL), NULL);
}

void PlayVideo(MemBuffer& vbuf) {
	AVFormatContext* fmt = avformat_alloc_context();
	unsigned char* iob = (unsigned char*)av_malloc(4096);
	fmt->pb = avio_alloc_context(iob, 4096, 0, &vbuf, ffmpeg_read, NULL, NULL);
	if (avformat_open_input(&fmt, NULL, NULL, NULL)) return;
	avformat_find_stream_info(fmt, NULL);

	int vs = -1;
	for (unsigned i = 0; i < fmt->nb_streams; i++)
		if (fmt->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) { vs = i; break; }
	if (vs == -1) return;

	AVCodecParameters* cp = fmt->streams[vs]->codecpar;
	const AVCodec* cdc = avcodec_find_decoder(cp->codec_id);
	AVCodecContext* ctx = avcodec_alloc_context3(cdc);
	avcodec_parameters_to_context(ctx, cp);
	avcodec_open2(ctx, cdc, NULL);

	SwsContext* sws = sws_getContext(ctx->width, ctx->height, ctx->pix_fmt,
		ctx->width, ctx->height, AV_PIX_FMT_BGR24, SWS_BILINEAR, NULL, NULL, NULL);

	double va = (double)ctx->width / ctx->height, sa = (double)g_w / g_h;
	int dw, dh, dx, dy;
	if (va > sa) { dw = g_w; dh = (int)(g_w / va); dx = 0; dy = (g_h - dh) / 2; }
	else { dh = g_h; dw = (int)(g_h * va); dx = (g_w - dw) / 2; dy = 0; }

	int nb = av_image_get_buffer_size(AV_PIX_FMT_BGR24, ctx->width, ctx->height, 1);
	uint8_t* rgb = (uint8_t*)av_malloc(nb);
	AVFrame* rf = av_frame_alloc();
	av_image_fill_arrays(rf->data, rf->linesize, rgb, AV_PIX_FMT_BGR24, ctx->width, ctx->height, 1);

	BITMAPINFO bmi = {};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = ctx->width;
	bmi.bmiHeader.biHeight = -ctx->height;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 24;

	AVPacket* pkt = av_packet_alloc();
	AVFrame* fr = av_frame_alloc();
	HDC hdc = GetDC(g_hwnd);
	int64_t t0 = av_gettime(), pts0 = -1;

	while (av_read_frame(fmt, pkt) >= 0) {
		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) goto done;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (pkt->stream_index == vs) {
			avcodec_send_packet(ctx, pkt);
			while (avcodec_receive_frame(ctx, fr) == 0) {
				if (pts0 < 0) pts0 = fr->pts;
				int64_t el = av_gettime() - t0, want = fr->pts - pts0;
				if (want > el) Sleep((DWORD)((want - el) / 1000));
				sws_scale(sws, fr->data, fr->linesize, 0, ctx->height, rf->data, rf->linesize);
				StretchDIBits(hdc, dx, dy, dw, dh, 0, 0, ctx->width, ctx->height,
					rf->data[0], &bmi, DIB_RGB_COLORS, SRCCOPY);
			}
		}
		av_packet_unref(pkt);
	}

done:
	ReleaseDC(g_hwnd, hdc);
	av_free(rgb);
	av_frame_free(&rf);
	av_frame_free(&fr);
	av_packet_free(&pkt);
	sws_freeContext(sws);
	avcodec_free_context(&ctx);
	avformat_close_input(&fmt);
	av_free(iob);
}

void PlayRickroll() {
	MemBuffer vb, ab;
	if (!LoadRes(IDR_VIDEO, vb)) return;
	if (!LoadRes(IDR_AUDIO, ab)) return;

	MemFLAC flac(&ab);
	flac.init();
	flac.process_until_end_of_stream();
	PlayAudio(flac.pcm, flac.rate, flac.ch, flac.bits);

	g_hwnd = MakeWindow();
	if (!g_hwnd) return;
	PlayVideo(vb);

	if (g_wo) {
		while (!(g_wh.dwFlags & WHDR_DONE)) Sleep(100);
		waveOutUnprepareHeader(g_wo, &g_wh, sizeof(WAVEHDR));
		waveOutClose(g_wo);
	}
	DestroyWindow(g_hwnd);
	ExitProcess(0);
}