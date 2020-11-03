package org.rokist.canlangtest

import android.app.Application

class MyApp : Application() {
    companion object {
        const val CONSTANT = 12
        lateinit var instance: MyApp
            private set
    }

    override fun onCreate() {
        super.onCreate()
        instance = this
        //        typeface = Typeface.createFromAsset(assets, "fonts/myFont.ttf")
    }

}