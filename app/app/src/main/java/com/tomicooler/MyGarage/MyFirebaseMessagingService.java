package com.tomicooler.MyGarage;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;

import androidx.annotation.NonNull;
import androidx.core.app.NotificationCompat;

import android.util.Log;

import com.google.firebase.messaging.FirebaseMessagingService;
import com.google.firebase.messaging.RemoteMessage;

import java.util.Objects;

public class MyFirebaseMessagingService extends FirebaseMessagingService {

    private static final String TAG = "MyFirebaseMsgService";

    @Override
    public void onMessageReceived(RemoteMessage remoteMessage) {
        Log.d(TAG, "From: " + remoteMessage.getFrom());

        if (remoteMessage.getData().size() > 0) {
            Log.d(TAG, "Message data payload: " + remoteMessage.getData());

            long timestamp = Long.parseLong(Objects.requireNonNull(remoteMessage.getData().getOrDefault("timestamp", "0")));
            Event.Action action = Event.parseAction(Objects.requireNonNull(remoteMessage.getData().getOrDefault("action", "")));

            if (timestamp != 0 && action != null) {
                Log.d(TAG, "ACTION " + timestamp + " " + action);
                EventRepository repository = new EventRepository(getApplicationContext());
                Event event = new Event(timestamp, action);
                repository.insert(event);
                sendNotification(String.format("%s %s", DateUtils.formatDate(event.getTimestamp()),
                        getString(event.getActionStringResource())));
            }
        }
    }


    @Override
    public void onNewToken(@NonNull String token) {
        sendNotification(getApplicationContext().getString(R.string.new_fcm_token));
    }


    private void sendNotification(String content) {
        Intent intent = new Intent(this, MainActivity.class);
        intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
        PendingIntent pendingIntent = PendingIntent.getActivity(this, 0, intent,
                PendingIntent.FLAG_IMMUTABLE);


        // Settings app
        //  Apps & notifications and then Notifications
        //  Choose Alerting
        // https://support.google.com/android/answer/9079661?hl=en#zippy=%2Cchoose-if-notifications-interrupt-you-or-stay-silent
        String channelId = getString(R.string.default_notification_channel_id);
        NotificationCompat.Builder notificationBuilder =
                new NotificationCompat.Builder(this, channelId)
                        .setSmallIcon(R.drawable.ic_baseline_notification_important_24)
                        .setContentTitle(getString(R.string.fcm_message))
                        .setContentText(content)
                        .setAutoCancel(true)
                        .setDefaults(Notification.DEFAULT_ALL)
                        .setContentIntent(pendingIntent);

        NotificationManager notificationManager =
                (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);

        NotificationChannel channel = new NotificationChannel(channelId,
                getString(R.string.default_notification_channel_name),
                NotificationManager.IMPORTANCE_HIGH);
        notificationManager.createNotificationChannel(channel);

        Notification notification = notificationBuilder.build();

        notificationManager.notify(0, notification);
    }
}