package org.rokist.canlangtest

import android.annotation.SuppressLint
import android.content.Context
import android.content.ContextWrapper
import android.content.Intent
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.Canvas
import android.os.Bundle
import android.util.Base64
import android.util.Log
import android.view.Menu
import android.view.MenuItem
import androidx.appcompat.app.AppCompatActivity
import androidx.databinding.DataBindingUtil
import com.caverock.androidsvg.SVG
import org.rokist.canlangtest.databinding.ActivityMainBinding


class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)


        // Collect tests
        val testEntryMap = mutableMapOf<String, TestEntry>()
        val topTestGroup = TestParser.collectTests(this, testEntryMap)

        // Apply theme: dark/light
        val themePref = this.getSharedPreferences("theme", Context.MODE_PRIVATE)
        val value = themePref.getString("theme", "dark")

        SharedModel.createInstance(
            value ?: "dark",
            testGroup = topTestGroup,
            testEntryMap = testEntryMap
        )

        binding = DataBindingUtil.setContentView(this, R.layout.activity_main)

        this.toActivity()
            ?.doWithText("abc\uD83D\uDC68\u200D\uD83D\uDC68\u200D\uD83D\uDC67\u200D\uD83D\uDC67{}[]_?><日本語@!test")
    }


    override fun onCreateOptionsMenu(menu: Menu?): Boolean {
        menuInflater.inflate(R.menu.action_menu, menu)

        return super.onCreateOptionsMenu(menu)
    }


    private fun setTheme(isDark: Boolean): Unit {
        val themePref = this.getSharedPreferences("theme", Context.MODE_PRIVATE)
        themePref
            .edit()
            .putString("theme", if (isDark) "dark" else "white")
            .commit()
    }


    fun isDarkTheme(): Boolean {
        val themePref = this.getSharedPreferences("theme", Context.MODE_PRIVATE)

        val value = themePref.getString("theme", "white")
        Log.d("aaa", "value = $value")
        return value == "dark"
    }


    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        when (item.itemId) {
            R.id.white_theme -> {
                setTheme(isDark = false)
                val intent = Intent(this, MainActivity::class.java)
                finish()
                startActivity(intent)
            }
            R.id.dark_theme -> {
                setTheme(isDark = true)

                val intent = Intent(this, MainActivity::class.java)
                finish()
                startActivity(intent)
            }

        }
        return super.onOptionsItemSelected(item)
    }


    // A native method that is implemented by the 'native-lib' native library,
    // which is packaged with this application.
    external fun stringFromJNI(): String

    external fun stringFromJNI2(l: Long): Long

    external fun runTest(testcasename: String, testname: String): Long

    external fun runTestNext(testIndex: Long): String

    external fun donothing(): Double

    external fun doWithText(str: String): Long

    companion object {
        // Used to load the 'native-lib' library on application startup.
        init {
            System.loadLibrary("native-lib")
        }

        fun convertBase64ToBitmap(b64: String): Bitmap? {
            val imageAsBytes: ByteArray = Base64.decode(b64.toByteArray(), Base64.DEFAULT)
            return BitmapFactory.decodeByteArray(imageAsBytes, 0, imageAsBytes.size)
        }


        fun getImage(context: Context, status: TestEntryStatus?): Bitmap? {

            val svgOK =
                "<?xml version=\"1.0\" ?><svg id=\"Layer_1\" style=\"enable-background:new 0 0 612 792;\" version=\"1.1\" viewBox=\"0 0 612 792\" xml:space=\"preserve\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\"><style type=\"text/css\">\n" +
                        "\t.st0{fill:#41AD49;}\n" +
                        "</style><g><path class=\"st0\" d=\"M562,396c0-141.4-114.6-256-256-256S50,254.6,50,396s114.6,256,256,256S562,537.4,562,396L562,396z    M501.7,296.3l-241,241l0,0l-17.2,17.2L110.3,421.3l58.8-58.8l74.5,74.5l199.4-199.4L501.7,296.3L501.7,296.3z\"/></g></svg>"

            val svgNG =
                "<?xml version=\"1.0\" ?><svg id=\"Layer_1\" style=\"enable-background:new 0 0 612 792;\" version=\"1.1\" viewBox=\"0 0 612 792\" xml:space=\"preserve\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\"><style type=\"text/css\">\n" +
                        "\t.st0{clip-path:url(#SVGID_2_);fill:none;stroke:#E44061;stroke-width:45;}\n" +
                        "\t.st1{fill:#E44061;}\n" +
                        "</style><g><g><defs><rect height=\"512\" id=\"SVGID_1_\" width=\"512\" x=\"50\" y=\"140\"/></defs><clipPath id=\"SVGID_2_\"><use style=\"overflow:visible;\" xlink:href=\"#SVGID_1_\"/></clipPath><path class=\"st0\" d=\"M306,629.5c128.8,0,233.5-104.7,233.5-233.5S434.8,162.5,306,162.5S72.5,267.2,72.5,396    S177.2,629.5,306,629.5L306,629.5z\"/></g><polygon class=\"st1\" points=\"348.7,396 448,296.7 405.3,254 306,353.3 206.7,254 164,296.7 263.3,396 164,495.3 206.7,538    306,438.7 405.3,538 448,495.3 348.7,396  \"/></g></svg>"

            val tt =
                "<?xml version=\"1.0\" ?><svg id=\"Layer_1\" style=\"enable-background:new 0 0 612 792;\" version=\"1.1\" viewBox=\"0 0 612 792\" xml:space=\"preserve\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\"><style type=\"text/css\">\n" +
                        "\t.st0{fill:#E44061;}\n" +
                        "</style><g><path class=\"st0\" d=\"M562,396c0-141.4-114.6-256-256-256S50,254.6,50,396s114.6,256,256,256S562,537.4,562,396L562,396z M356.8,396   L475,514.2L424.2,565L306,446.8L187.8,565L137,514.2L255.2,396L137,277.8l50.8-50.8L306,345.2L424.2,227l50.8,50.8L356.8,396   L356.8,396z\"/></g></svg>"


            val svgOK2 =
                "<?xml version=\"1.0\" ?><svg id=\"Layer_1\" style=\"enable-background:new 0 0 512 512;\" version=\"1.1\" viewBox=\"0 0 512 512\" xml:space=\"preserve\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\"><style type=\"text/css\">\n" +
                        "\t.st0{fill:#41AD49;}\n" +
                        "</style><g><polygon class=\"st0\" points=\"434.8,49 174.2,309.7 76.8,212.3 0,289.2 174.1,463.3 196.6,440.9 196.6,440.9 511.7,125.8 434.8,49     \"/></g></svg>"

            val svgText = when (status) {
                TestEntryStatus.Succeeded -> svgOK
                TestEntryStatus.Failed -> svgNG
                else -> ""
            }
            try {
                val assetManager = context.assets;
                val svg = SVG.getFromString(svgText)
                val bitmap = Bitmap.createBitmap(100, 100, Bitmap.Config.ARGB_8888);

                val canvas = Canvas(bitmap);
                svg.renderToCanvas(canvas);

                return bitmap
            } catch (e: java.lang.Exception) {

            }

            return null;//convertBase64ToBitmap(tt)
        }
    }
}


class LockObj {

}

fun getLockObject(): LockObj {

    return LockObj()
}

tailrec fun Context.activity(): MainActivity? = when {
    this is MainActivity -> this
    else -> (this as? ContextWrapper)?.baseContext?.activity()
}

fun Context.toActivity(): MainActivity? {
    var context = this
    while (context is ContextWrapper) {
        if (context is MainActivity) {
            return context
        }
        context = context.baseContext
    }
    return null
}


// コンパイル時にすべてのロックオブジェクトを
// 解析できないといけない?

/*

file A: lock default_lock_a
    per-thread no guard vars:
    shared vars: a_var1, a_var2

file B: lock default_lock_b
    per-thread no guard  vars:
    shared vars: b_var1, b_var2


class SampleClass {
    lock a {
        public var avar;
        private var bvar;
    }

    lock b {
        private var avar;
        private var bvar;
    }

    private {
        var  avar;
        var bvar;
    }

    init {

    }

    private int a;
    private int b;
}

     int a = 54;
     int c = a;
     auto g = c * a
*/

/*
Obj obj = new Obj();
parallel-for (0..1000) {
    obj.method1()
}
*/

