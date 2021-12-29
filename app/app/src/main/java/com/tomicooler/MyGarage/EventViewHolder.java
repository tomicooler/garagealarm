package com.tomicooler.MyGarage;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

public class EventViewHolder extends RecyclerView.ViewHolder {

    private final TextView eventItemView;

    public EventViewHolder(@NonNull View itemView) {
        super(itemView);
        eventItemView = itemView.findViewById(R.id.textView);
    }

    public void bind(Event event) {
        eventItemView.setText(event.getActionStringResource());
    }

    static EventViewHolder create(ViewGroup parent) {
        View view = LayoutInflater.from(parent.getContext())
                .inflate(R.layout.recyclerview_item, parent, false);
        return new EventViewHolder(view);
    }
}
