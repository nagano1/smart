package org.rokist.canlangtest

import android.graphics.Color
import androidx.lifecycle.MutableLiveData

enum class Theme {
    Dark,
    White
}

// Represents a shared model used through this app
class SharedModel(
    val theme: Theme,
    topTestGroup: TestGroup,
    testEntryMap2: Map<String, TestEntry>
) {
    val selected = MutableLiveData<Boolean>()

    val testEntryMap: Map<String, TestEntry>

    init {
        this.selected.value = false
        this.testEntryMap = testEntryMap2
    }

    private fun darkTheme(): Boolean {
        return theme == Theme.Dark
    }

    val commonBackgroundColor: Int
        get() {
            return when (darkTheme()) {
                true -> Color.BLACK
                false -> Color.WHITE
            }
        }

    val commonForegroundColor: Int
        get() {
            return when (darkTheme()) {
                true -> Color.WHITE
                false -> Color.BLACK
            }
        }
    val secondaryForegroundColor: Int
        get() {
            return when (darkTheme()) {
                true -> Color.GRAY
                false -> Color.DKGRAY
            }
        }


    val tabHiddenTextColor: Int
        get() {
            return when (darkTheme()) {
                true -> Color.parseColor("#727272")
                false -> Color.parseColor("#AAAAAA")
            }
        }

    var topTestGroup: TestGroup = topTestGroup
    var currentTestItem: ITestItem = topTestGroup

    fun select(item: Boolean) {
        selected.postValue(item)
    }

    companion object {
        var mInstance: SharedModel? = null
        fun createInstance(
            theme: String,
            testGroup: TestGroup,
            testEntryMap: Map<String, TestEntry>
        ) {
            val themeClass = if (theme == "dark") {
                Theme.Dark
            } else {
                Theme.White
            }
            mInstance = SharedModel(
                themeClass,
                testGroup,
                testEntryMap
            )
        }

        fun instance(): SharedModel {
            return mInstance!!
        }
    }
}