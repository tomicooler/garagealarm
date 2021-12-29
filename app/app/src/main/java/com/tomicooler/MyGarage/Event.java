package com.tomicooler.MyGarage;

import androidx.annotation.NonNull;
import androidx.room.ColumnInfo;
import androidx.room.Entity;
import androidx.room.PrimaryKey;

@Entity(tableName = "events")
public class Event {
    public void setId(long id) {
        this.id = id;
    }

    enum Action {
        Open,
        Close,
        IvTopMismatch,
        IvBottomOld
    }

    @PrimaryKey(autoGenerate = true)
    @ColumnInfo(name = "id")
    private long id;

    @ColumnInfo(name = "timestamp")
    final private long timestamp;

    @ColumnInfo(name = "action")
    final private Action action;

    public Event(long timestamp, Action action) {
        this.timestamp = timestamp;
        this.action = action;
    }

    public long getId() {
        return id;
    }

    public long getTimestamp() {
        return timestamp;
    }

    public Action getAction() {
        return action;
    }

    public static Action parseAction(@NonNull String string) {
        if (string.equals("open")) return Action.Open;
        if (string.equals("close")) return Action.Close;
        if (string.equals("iv_top_mismatch")) return Action.IvTopMismatch;
        if (string.equals("iv_bottom_old")) return Action.IvBottomOld;
        return null;
    }

    public int getActionStringResource() {
        switch (action) {
            case Open:
                return R.string.action_open;
            case Close:
                return R.string.action_close;
            case IvTopMismatch:
                return R.string.action_iv_top_mismatch;
            case IvBottomOld:
                return R.string.iv_bottom_old;
        }

        return R.string.app_name;
    }
}
