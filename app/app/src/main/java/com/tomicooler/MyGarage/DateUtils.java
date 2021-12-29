package com.tomicooler.MyGarage;

import java.text.DateFormat;
import java.util.Date;

public class DateUtils {
    public static String formatDate(long timestamp) {
        return DateFormat.getDateTimeInstance(DateFormat.SHORT, DateFormat.SHORT).format(new Date(timestamp));
    }
}
