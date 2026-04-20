/*
 * 任务bean.
 *  * @author jdpxiaoming
 *  * @github https://github.com/jdpxiaoming
 *  * created  20-5-21 下午2:07: $time
 */
package com.jdpxiaoming.ffmpeg_cmd;

public class FFmepgTask {

    private long taskId;
    private long duration;
    private String[] cmds;
    public FFmpegUtil.onCallBack callBacklistener;

    public FFmepgTask(long taskId, long duration, String[] cmds ,FFmpegUtil.onCallBack listener) {
        this.taskId = taskId;
        this.duration = duration;
        this.cmds = cmds;
        this.callBacklistener = listener;
    }

    public long getDuration() {
        return duration;
    }

    public void setDuration(long duration) {
        this.duration = duration;
    }

    public long getTaskId() {
        return taskId;
    }

    public void setTaskId(long taskId) {
        this.taskId = taskId;
    }

    public String[] getCmds() {
        return cmds;
    }

    public void setCmds(String[] cmds) {
        this.cmds = cmds;
    }
}
