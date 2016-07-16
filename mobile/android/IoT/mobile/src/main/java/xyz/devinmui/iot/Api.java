package xyz.devinmui.iot;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.IOException;

import okhttp3.Call;
import okhttp3.Callback;
import okhttp3.MediaType;
import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.RequestBody;
import okhttp3.Response;

/**
 * Created by devinmui on 7/13/16.
 */
public class Api {

    final String url = "http://192.168.56.1:3000";

    public static final MediaType JSON
            = MediaType.parse("application/json");

    OkHttpClient client = new OkHttpClient();

    Call get(String uri, Callback callback) {
        Request request = new Request.Builder()
                .url(getUrl(uri))
                .build();

        Call response = client.newCall(request);
        response.enqueue(callback);
        return response;
    }

    Call post(String uri, String json, Callback callback) throws IOException {
        RequestBody body = RequestBody.create(JSON, json);
        Request request = new Request.Builder()
                .url(getUrl(uri))
                .addHeader("Accept", "application/json")
                .addHeader("Content-Type", "application/json")
                .post(body)
                .build();
        Call call = client.newCall(request);
        call.enqueue(callback);
        return call;
    }

    String getUrl(String uri){
        return url + uri;
    }
}
