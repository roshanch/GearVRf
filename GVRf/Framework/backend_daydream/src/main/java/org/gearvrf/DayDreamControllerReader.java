package org.gearvrf;

import android.graphics.PointF;

import org.joml.Quaternionf;
import org.joml.Vector3f;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

import com.google.vr.sdk.controller.Controller;
import com.google.vr.sdk.controller.ControllerManager;
import com.google.vr.sdk.controller.Orientation;

class DayDreamControllerReader implements GearCursorController.ControllerReader {


    private ControllerManager mControllerManager;
    private Controller mController;

    DayDreamControllerReader(GVRContext gvrContext) {
        EventListener listener = new EventListener();
        mControllerManager = new ControllerManager(gvrContext.getContext(), listener);

        mController = mControllerManager.getController();
        mController.setEventListener(listener);
        mControllerManager.start();
       // GvrAr
    }

    @Override
    public boolean isConnected() {

    }

    @Override
    public void updateRotation(Quaternionf quat) {
        Orientation orientation = mController.orientation;
        quat.set(orientation.x, orientation.y, orientation.z,orientation.w);
    }

    @Override
    public void updatePosition(Vector3f vec) {
        vec.set(mController.position[0], mController.position[1], mController.position[2]);
    }

    @Override
    public int getKey() {
        if(mController.appButtonState)
            return GearCursorController.CONTROLLER_KEYS.BUTTON_A.getNumVal();
        if(mController.clickButtonState)
            return GearCursorController.CONTROLLER_KEYS.BUTTON_ENTER.getNumVal();
        if(mController.volumeUpButtonState)
            return GearCursorController.CONTROLLER_KEYS.BUTTON_VOLUME_UP.getNumVal();
        if(mController.volumeDownButtonState)
            return GearCursorController.CONTROLLER_KEYS.BUTTON_VOLUME_DOWN.getNumVal();
        if(mController.homeButtonState)
            return GearCursorController.CONTROLLER_KEYS.BUTTON_HOME.getNumVal();

        return 0;
    }

    @Override
    public float getHandedness() {

    }

    @Override
    public void updateTouchpad(PointF pt) {
        pt.set(mController.touch.x, mController.touch.y);
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            mControllerManager.stop();
            mControllerManager = null;
        } finally {
            super.finalize();
        }
    }
    private class EventListener extends Controller.EventListener
            implements ControllerManager.EventListener {

        @Override
        public void onApiStatusChanged(int i) {
        }

        @Override
        public void onRecentered() {
        }

        @Override
        public void onUpdate() {
            mController.update();
        }
    }
}
