package com.tomicooler.MyGarage;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

public class EventViewHolder extends RecyclerView.ViewHolder {

    private final TextView eventText;
    private final TextView eventTimestamp;
    private final ImageView eventIcon;

    public EventViewHolder(@NonNull View itemView) {
        super(itemView);
        eventText = itemView.findViewById(R.id.eventText);
        eventTimestamp = itemView.findViewById(R.id.eventTimestamp);
        eventIcon = itemView.findViewById(R.id.eventIcon);
    }

    public void bind(Event event) {
        eventText.setText(event.getActionStringResource());
        eventTimestamp.setText(DateUtils.formatDate(event.getTimestamp()));
        switch (event.getAction()) {
            case Open:
                eventIcon.setImageResource(R.mipmap.ic_launcher);
                break;
            case Close:
                eventIcon.setImageResource(R.mipmap.garage_closed);
                break;
            default:
                eventIcon.setImageResource(R.drawable.ic_baseline_notification_important_24);
                break;
        }
    }

    static EventViewHolder create(ViewGroup parent) {
        View view = LayoutInflater.from(parent.getContext())
                .inflate(R.layout.recyclerview_item, parent, false);
        return new EventViewHolder(view);
    }
}
