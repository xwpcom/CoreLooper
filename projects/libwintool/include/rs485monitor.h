#pragma once

using namespace Bear::Core;

struct tagSensorItem
{
    BYTE    addr = 0;
    string  title;
};

/*
XiongWanPing 2021.09.02
*/
class WIN_CLASS RS485Monitor
{
public:
    RS485Monitor();
    void OnCreate();
    void Input(LPBYTE d, int bytes);

    void LoadConfig();
    tagSensorItem *item(BYTE addr);
    sigslot::signal2<LPBYTE, int> SignalFrameReady;
    sigslot::signal2<LPBYTE, int> SignalUnknownDataReady;
protected:
    void ParseBox(ByteBuffer& box);
    void OnParsedFrame(LPBYTE frame, int bytes);
    void OnParsedUnknown(LPBYTE data, int bytes);
    unordered_map<BYTE, tagSensorItem> mItems;//addr

};
