package com.tomicooler.MyGarage;

import android.content.Context;

import androidx.lifecycle.LiveData;

import java.util.List;

public class EventRepository {
    private final EventDao mEventDao;
    private final LiveData<List<Event>> mEvents;

    // TODO: remove application for unit-test ability
    EventRepository(Context context) {
        EventRoomDatabase db = EventRoomDatabase.getDatabase(context);
        mEventDao = db.eventDao();
        mEvents = mEventDao.getEvents();
    }

    LiveData<List<Event>> getAllWords() {
        return mEvents;
    }

    void insert(Event event) {
        EventRoomDatabase.databaseWriteExecutor.execute(() -> {
            mEventDao.insert(event);
        });
    }

    void deleteAll() {
        EventRoomDatabase.databaseWriteExecutor.execute(mEventDao::deleteAll);
    }
}
