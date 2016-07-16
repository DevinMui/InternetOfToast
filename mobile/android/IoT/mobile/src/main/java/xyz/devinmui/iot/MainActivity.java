package xyz.devinmui.iot;

import android.content.Context;
import android.content.DialogInterface;
import android.os.AsyncTask;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.RadioGroup;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.IOException;

import okhttp3.Call;
import okhttp3.Callback;
import okhttp3.Response;

public class MainActivity extends AppCompatActivity {
    Api api = new Api();
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
    }

    public void toast(View v) throws IOException {
        final Context context = v.getContext();
        EditText amountText = (EditText) findViewById(R.id.amountText);
        RadioGroup group = (RadioGroup) findViewById(R.id.radioGroup);
        int amount = Integer.parseInt(String.valueOf(amountText.getText()));
        if(amount <= 0){
            System.out.println("Too little bread");
        } else {
            int g = group.getCheckedRadioButtonId();
            RadioButton colorButton = (RadioButton) findViewById(g);
            String color = (String) colorButton.getText();
            if(color.equals("Light") || color.equals("Medium") || color.equals("Dark")){
                String json = "{\"amount\": " + amount + ", \"color\": \"" + color.toLowerCase() + "\"}";

                api.post("/toast", json, new Callback() {

                    @Override
                    public void onFailure(Call call, IOException e) {
                        e.printStackTrace();
                    }

                    @Override
                    public void onResponse(Call call, Response response) throws IOException {
                        String res = response.body().string();
                        try {
                            final JSONObject jsonThing = new JSONObject(res);
                            runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    new AlertDialog.Builder(context)
                                            .setTitle("We're making toast now!")
                                            .setMessage("We'll let you know when it's done!")
                                            .setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {
                                                public void onClick(DialogInterface dialog, int which) {
                                                    CheckToastTask task = new CheckToastTask(context);
                                                    task.execute();
                                                }
                                            })
                                            .setIcon(android.R.drawable.ic_dialog_info)
                                            .show();
                                }
                            });

                            // great! maybe start a timer too?
                        } catch (JSONException e) {
                            e.printStackTrace();
                            runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    new AlertDialog.Builder(context)
                                            .setTitle("We're already making toast...")
                                            .setMessage("We'll let you know when it's done!")
                                            .setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {
                                                public void onClick(DialogInterface dialog, int which) {
                                                    // continue with delete
                                                }
                                            })
                                            .setIcon(android.R.drawable.ic_dialog_alert)
                                            .show();
                                }
                            });
                        }
                    }
                });
            } else {
                // return error
                System.out.println("Somehow you hacked this thing");
            }
        }
    }

    private class CheckToastTask extends AsyncTask<String, Void, String> {
        private Context mContext;

        public CheckToastTask (Context context){
            mContext = context;
        }
        @Override
        protected String doInBackground(String... params) {
            final Boolean[] checkToast = {true};
            while(true){
                if(checkToast[0]) {
                    // check for progress
                    api.get("/toast_progress", new Callback() {
                        @Override
                        public void onFailure(Call call, IOException e) {

                        }

                        @Override
                        public void onResponse(Call call, Response response) throws IOException {
                            try {
                                JSONObject json = new JSONObject(response.body().string());
                                // check if progress is false
                                if (!json.getBoolean("progress")) {
                                    checkToast[0] = false;
                                    runOnUiThread(new Runnable() {
                                        @Override
                                        public void run() {
                                            new AlertDialog.Builder(mContext)
                                                    .setTitle("The toast is done!")
                                                    .setMessage("Go get it now!")
                                                    .setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {
                                                        public void onClick(DialogInterface dialog, int which) {
                                                            // continue with delete
                                                        }
                                                    })
                                                    .setIcon(android.R.drawable.ic_dialog_info)
                                                    .show();
                                        }
                                    });
                                } else if(json.getBoolean("progress")) {
                                    checkToast[0] = true;
                                }
                            } catch (JSONException e) {
                                e.printStackTrace();
                            }
                        }
                    });
                }
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }

        @Override
        protected void onPostExecute(String result) {
        }

        @Override
        protected void onPreExecute() {}

        @Override
        protected void onProgressUpdate(Void... values) {}
    }
}
