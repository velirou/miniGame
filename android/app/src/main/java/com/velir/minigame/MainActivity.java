package com.velir.minigame;

import android.app.NativeActivity;
import android.content.pm.ActivityInfo;
import android.os.Bundle;

public class MainActivity extends NativeActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_SENSOR_LANDSCAPE);
        super.onCreate(savedInstanceState);
    }
}
