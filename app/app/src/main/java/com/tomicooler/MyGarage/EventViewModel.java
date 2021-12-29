package com.tomicooler.MyGarage;

import android.app.Application;

import androidx.annotation.NonNull;
import androidx.lifecycle.AndroidViewModel;
import androidx.lifecycle.LiveData;

import java.util.List;

public class EventViewModel extends AndroidViewModel {

    private final EventRepository mRepository;
    private final LiveData<List<Event>> mEvents;

    public EventViewModel(@NonNull Application application) {
        super(application);
        mRepository = new EventRepository(application);
        mEvents = mRepository.getAllWords();
    }

    public LiveData<List<Event>> getEvents() {
        return mEvents;
    }

    public void deleteAll() {
        mRepository.deleteAll();
    }
}
