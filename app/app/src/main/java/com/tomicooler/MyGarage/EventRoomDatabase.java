package com.tomicooler.MyGarage;

import android.content.Context;

import androidx.room.Database;
import androidx.room.Room;
import androidx.room.RoomDatabase;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

// TODO: exportSchema
@Database(entities = {Event.class}, version = 1, exportSchema = false)
public abstract  class EventRoomDatabase extends RoomDatabase {
    public abstract EventDao eventDao();

    private static volatile EventRoomDatabase INSTANCE;
    private static final int NUMBER_OF_THREADS = 4;
    static final ExecutorService databaseWriteExecutor =
            Executors.newFixedThreadPool(NUMBER_OF_THREADS);

    static EventRoomDatabase getDatabase(final Context context) {
        if (INSTANCE == null) {
            synchronized (EventRoomDatabase.class) {
                if (INSTANCE == null) {
                    INSTANCE = Room.databaseBuilder(context.getApplicationContext(),
                            EventRoomDatabase.class, "event_database")
                            .build();
                }
            }
        }
        return INSTANCE;
    }
}
