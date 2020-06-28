#include <jni.h>
#include <string>
#include <android/log.h>
#include <android/native_window_jni.h>
#include <unistd.h>


const char * TAG = "llk";


extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_llk_ff_FFPlayer_getAvCodecVersion(
        JNIEnv *env,
        jobject /* this */) {
    return env->NewStringUTF(av_version_info());
}

/**
 * 全过程
 * 1、解封装 拿到音视频上下文
 * 2、遍历流 拿到对应的音频流、视频流
 * 3、解码器上下文
 * 4、获取到解码器
 * 5、从流中读取packet
 * 6、packet转换成frame
 * 7、统一转换成可显示的显示格式
 * 8、输出到对应的响应设备
 */
extern "C" JNIEXPORT void JNICALL
Java_com_llk_ff_FFPlayer_playFromNative(
        JNIEnv *env,
        jobject instance,
        jstring path_,
        jobject surface) {

    ANativeWindow *window = ANativeWindow_fromSurface(env, surface);

    const char *path = env->GetStringUTFChars(path_, 0);

    //初始化网络模块
    avformat_network_init();

    //音视频上下文
    AVFormatContext *format_ctx = avformat_alloc_context();

    AVDictionary *options = NULL;
    av_dict_set(&options, "timeout", "30000000", 0);
    //通过传入文件路径与配置参数，解码获取音视频上下文。
    int errorCode = avformat_open_input(&format_ctx, path, NULL, &options);
    if (errorCode){ //TODO 返回错误码，0为无错误
        __android_log_print(ANDROID_LOG_ERROR, TAG, "avformat_open_input fail, error=%d", errorCode);
        return;
    }

    //====== 获取 视频流 参数信息 ======
    AVCodecParameters *video_parm = NULL;
    int vedio_stream_idx=-1;

    //====== 获取 音频流 参数信息 ======


    avformat_find_stream_info(format_ctx, NULL);
    for (int i = 0; i < format_ctx->nb_streams; ++i) {
        int type = format_ctx->streams[i]->codecpar->codec_type;
        switch (type){
            case AVMEDIA_TYPE_VIDEO:
                video_parm = format_ctx->streams[i]->codecpar;
                vedio_stream_idx = i;
                break;
            case AVMEDIA_TYPE_AUDIO:

                break;
            default:
                break;
        }
    }

    //获取视频解码器
    AVCodec *video_codec = NULL;
    if(NULL != video_parm) video_codec = avcodec_find_decoder(video_parm->codec_id);
    //获取解码器上下文
    AVCodecContext *codec_ctx = avcodec_alloc_context3(video_codec);
    //将视频流参数拷贝到解码器上下文
    avcodec_parameters_to_context(codec_ctx, video_parm);

    avcodec_open2(codec_ctx, video_codec, NULL);
    //解码yuv数据
    //AVPacket malloc() 相当于 new AVPacket()
    //所有ffmpeg的对象构建都是通过内置的函数构建的。
    AVPacket *video_packet = av_packet_alloc();


    SwsContext *sws_ctx = sws_getContext(codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt,
                   codec_ctx->width, codec_ctx->height, AV_PIX_FMT_ARGB, //转换成argb
                   SWS_BICUBLIN,
                   0,
                   0,
                   0);


    //ANativeWindow绘制的原理，通过内置的buffer缓存区进行绘制。
    //设置好一个固定的缓冲区之后，通过然后不断往buffer里边更新像素数据来进行刷新。
    //拿到首地址，整行刷新像素数据。

    //设置window缓存区大小
    ANativeWindow_setBuffersGeometry(window, codec_ctx->width, codec_ctx->height,
            WINDOW_FORMAT_RGBA_8888);

    ANativeWindow_Buffer outBuffer;

    //不断获取视频数据
    while (av_read_frame(format_ctx, video_packet) >= 0){
        avcodec_send_packet(codec_ctx, video_packet);

        //每一帧的数据
        AVFrame *frame = av_frame_alloc();
        int ret = avcodec_receive_frame(codec_ctx, frame);
        if (AVERROR(EAGAIN) == ret){ //没有不可用数据，继续去取
            continue;
        } else if (ret < 0){ //读到视频末尾，直接返回
            break;
        }

        //存储r g b a 数据输出容器
        uint8_t *dst_data[4];
        //存储r g b a 每一行的首地址
        int dst_linesize[4];

        //初始化容器大小
        av_image_alloc(dst_data,
                dst_linesize,
                codec_ctx->width,
                codec_ctx->height,
                AV_PIX_FMT_RGBA,
                1);

        //绘制
        //将 每一帧 yuv数据 转换成 argb数据
        sws_scale(sws_ctx, frame->data, frame->linesize, 0, frame->height,
                dst_data, dst_linesize);

        if (video_packet->stream_index == vedio_stream_idx) {
            //非零   正在解码
            if (ret==0) {
                //渲染
                ANativeWindow_lock(window, &outBuffer, NULL); //window上锁

                uint8_t *f_window = static_cast<uint8_t  *>(outBuffer.bits);
                uint8_t *src_data = dst_data[0]; //输入源rgb
                int dest_stride = outBuffer.stride * 4; //拿到一行有多少个rgba字节
                int src_linesize = dst_linesize[0];

                for (int i = 0; i < outBuffer.height; ++i) {
                    memcpy(f_window + i * dest_stride, src_data + i * src_linesize, dest_stride); //内存拷贝来进行渲染
                }

                ANativeWindow_unlockAndPost(window); //解锁

                usleep(1000 * 16);

                av_frame_free(&frame);
            }
        }
    }

    ANativeWindow_release(window);
    avcodec_close(codec_ctx);
    avformat_free_context(format_ctx);

    env->ReleaseStringUTFChars(path_, path);
}
