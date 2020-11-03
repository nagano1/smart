package org.rokist.canlangtest

import android.content.Context
import android.util.Log
import org.json.JSONArray
import java.io.ByteArrayOutputStream
import java.io.IOException
import java.io.InputStream

class TestParser {
    companion object {
        fun collectTests(context: Context, dict:MutableMap<String, TestEntry>): TestGroup {
            val topTestGroup = TestGroup(
                "Top"
            )
            topTestGroup.isTop = true

            val inputStream: InputStream = context.resources.openRawResource(R.raw.testlist)
            val byteArrayOutputStream = ByteArrayOutputStream()

            var ctr: Int = -2
            try {
                while (ctr != -1) {
                    if (ctr > -1) byteArrayOutputStream.write(ctr);
                    ctr = inputStream.read()
                }
                inputStream.close()
            } catch (e: IOException) {
                e.printStackTrace()
            }

            try {
                val jArray = JSONArray(byteArrayOutputStream.toString())

                val speedGroup = TestGroup("SpeedTest")
                topTestGroup.addTest(speedGroup)

                val parserGroup =  TestGroup("Parser Tests")
                topTestGroup.addTest(parserGroup)


                for (i in 0 until jArray.length()) {
                    val itemEntry = jArray.getJSONObject(i)
                    val testCaseName = itemEntry.getString("testcasename")
                    val testName = itemEntry.getString("testname")
                    val filepath = itemEntry.getString("filepath")
                    // tests\cplusplus_test.cpp
                    val filename = itemEntry.getString("filename")
                    val body = itemEntry.getString("body")
                    val testEntry = TestEntry(
                        testCaseName,
                        testName,
                        filepath,
                        filename,
                        body
                    )


                    if (testCaseName.startsWith("SpeedTest")) {
                        speedGroup.addTest(testEntry)
                    } else if (filename.startsWith("parser_test")) {
                        parserGroup.addTest(testEntry)
                    } else {
                        topTestGroup.addTest(testEntry)
                    }
                    dict[testEntry.name] = testEntry
                    //Log.v("aaa", testEntry.toString())
                }
            } catch (e: Exception) {
                e.printStackTrace()
            }

            return topTestGroup
        }
    }
}