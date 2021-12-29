package com.tomicooler.MyGarage;

import android.view.ViewGroup;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.DiffUtil;
import androidx.recyclerview.widget.ListAdapter;

public class EventListAdapter extends ListAdapter<Event, EventViewHolder> {

    protected EventListAdapter(@NonNull DiffUtil.ItemCallback<Event> diffCallback) {
        super(diffCallback);
    }

    @NonNull
    @Override
    public EventViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        return EventViewHolder.create(parent);
    }

    @Override
    public void onBindViewHolder(@NonNull EventViewHolder holder, int position) {
        Event current = getItem(position);
        holder.bind(current);
    }

    static class EventDiff extends DiffUtil.ItemCallback<Event> {
        @Override
        public boolean areItemsTheSame(@NonNull Event oldItem, @NonNull Event newItem) {
            return oldItem.getId() == newItem.getId();
        }

        @Override
        public boolean areContentsTheSame(@NonNull Event oldItem, @NonNull Event newItem) {
            return oldItem.getAction().equals(newItem.getAction()) && oldItem.getTimestamp() == newItem.getTimestamp();
        }
    }
}
