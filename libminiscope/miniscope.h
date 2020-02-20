/*
 * Copyright (C) 2019-2020 Matthias Klumpp <matthias@tenstral.net>
 *
 * Licensed under the GNU Lesser General Public License Version 3
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the license, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this software.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MINISCOPE_H
#define MINISCOPE_H

#include <memory>
#include <functional>
#include <opencv2/core.hpp>

#include "mediatypes.h"

#ifdef _WIN32
#define MS_LIB_EXPORT __declspec(dllexport)
#else
#define MS_LIB_EXPORT __attribute__((visibility("default")))
#endif

namespace MScope
{

using steady_hr_clock =
    std::conditional<std::chrono::high_resolution_clock::is_steady,
                     std::chrono::high_resolution_clock,
                     std::chrono::steady_clock
                    >::type;

using milliseconds_t = std::chrono::milliseconds;

enum class BackgroundDiffMethod {
    None,
    Subtraction,
    Division
};

class MiniScopeData;
class MS_LIB_EXPORT MiniScope
{
public:
    explicit MiniScope();
    ~MiniScope();

    void setScopeCamId(int id);
    int scopeCamId() const;

    void setExposure(double value);
    double exposure() const;

    void setGain(double value);
    double gain() const;

    void setExcitation(double value);
    double excitation() const;

    bool connect();
    void disconnect();

    bool run();
    void stop();
    bool startRecording(const std::string& fname = "");
    void stopRecording();

    bool running() const;
    bool recording() const;

    void setOnMessage(std::function<void(const std::string&, void*)> callback, void *udata = nullptr);
    void setPrintMessagesToStdout(bool enabled);

    bool useColor() const;
    void setUseColor(bool color);

    void setVisibleChannels(bool red, bool green, bool blue);
    bool showRedChannel() const;
    bool showGreenChannel() const;
    bool showBlueChannel() const;

    /**
     * @brief Called *in the DAQ thread* when a frame was acquired on the original acquired frame.
     *
     * This callback is executed for each raw frame acquired from the Miniscope, and is equivalent
     * to what would be recorded to a video file.
     */
    void setOnFrame(std::function<void(const cv::Mat &, const milliseconds_t &, void *)> callback, void *udata = nullptr);

    /**
     * @brief Called *in the DAQ thread* when a frame was acquired on the edited frame.
     *
     * This callback is executed for each possibly modified "display frame" that an application like
     * PoMiDAQ would show to the user.
     */
    void setOnDisplayFrame(std::function<void(const cv::Mat &, const milliseconds_t &, void *)> callback, void *udata = nullptr);

    cv::Mat currentDisplayFrame();
    uint currentFps() const;
    size_t droppedFramesCount() const;

    uint fps() const;
    void setFps(uint fps);

    void setCaptureStartTimepoint(std::chrono::time_point<steady_hr_clock> timepoint);
    bool useUnixTimestamps() const;
    void setUseUnixTimestamps(bool useUnixTime);
    milliseconds_t unixCaptureStartTime() const;

    bool externalRecordTrigger() const;
    void setExternalRecordTrigger(bool enabled);

    std::string videoFilename() const;
    void setVideoFilename(const std::string& fname);

    VideoCodec videoCodec() const;
    void setVideoCodec(VideoCodec codec);

    VideoContainer videoContainer() const;
    void setVideoContainer(VideoContainer container);

    bool recordLossless() const;
    void setRecordLossless(bool lossless);

    int minFluorDisplay() const;
    void setMinFluorDisplay(int value);

    int maxFluorDisplay() const;
    void setMaxFluorDisplay(int value);

    int minFluor() const;
    int maxFluor() const;

    BackgroundDiffMethod displayBgDiffMethod() const;
    void setDisplayBgDiffMethod(BackgroundDiffMethod method);

    double bgAccumulateAlpha() const;
    void setBgAccumulateAlpha(double value);

    uint recordingSliceInterval() const;
    void setRecordingSliceInterval(uint minutes);

    std::string lastError() const;

    milliseconds_t lastRecordedFrameTime() const;

private:
    MiniScopeData *d;

    void setLed(double value);
    void addFrameToBuffer(const cv::Mat& frame, const milliseconds_t &timestamp);
    std::chrono::time_point<steady_hr_clock> calculateCaptureStartTime(std::chrono::time_point<steady_hr_clock> firstFrameTime);
    static void captureThread(void *msPtr);
    void startCaptureThread();
    void finishCaptureThread();
    void emitMessage(const std::string& msg);
    void fail(const std::string& msg);
};

} // end of MiniScope namespace

#endif // MINISCOPE_H
