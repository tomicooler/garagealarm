package com.tomicooler.MyGarage;

import androidx.lifecycle.LiveData;
import androidx.room.Dao;
import androidx.room.Insert;
import androidx.room.Query;

import java.util.List;

@Dao
public interface EventDao {
    @Insert()
    void insert(Event event);

    @Query("DELETE FROM events")
    void deleteAll();

    @Query("SELECT * FROM events ORDER BY timestamp DESC")
    LiveData<List<Event>> getEvents();
}
