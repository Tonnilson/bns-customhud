struct World
{
    char unknown_0[0x50];
    bool _activated;
    bool _IsTerrainChanged;
    bool _isTransit;
    bool _isEnterWorld;
    bool _isEnterZone;
    bool _tryLeaveZone;
    char _leaveReason;
    char unknown_1[1];
    short _worldId;
    char unknown_2[6];
    __int64 _zoneId;
    int _geozoneId;
};

World* (__fastcall* BNSClient_GetWorld)();
PresentationWorld* (__fastcall* BNSClient_GetPresentationWorld)();
