package org.rokist.canlangtest

import android.util.Log
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.Observer


interface ITestItem {
    val name: String
    var status: MutableLiveData<TestEntryStatus>
}


data class TestGroup(
    override val name: String
) : ITestItem {
    var isTop: Boolean = false
    override var status: MutableLiveData<TestEntryStatus> = MutableLiveData(TestEntryStatus.NotTestedYet)
    private val _mutableTests = mutableListOf<ITestItem>()
    val tests: List<ITestItem> get() = _mutableTests

    private fun updateStatus() {
        var failed = false
        var isTesting = false
        var allNotYet = true
        for (test in tests) {
            when (test.status.value) {
                TestEntryStatus.Failed -> failed = true
                TestEntryStatus.IsTesting -> isTesting = true
                TestEntryStatus.NotTestedYet -> {}
                else -> allNotYet = false
            }
        }

        if (isTesting) {
            this.status.postValue( TestEntryStatus.IsTesting )
        } else if (failed) {
            this.status.postValue( TestEntryStatus.Failed)
        } else if (allNotYet) {
            this.status.postValue( TestEntryStatus.NotTestedYet )
        } else {
            this.status.postValue(TestEntryStatus.Succeeded)
        }
    }

    fun addTest(test: ITestItem) {
        test.status.observeForever( Observer {
            updateStatus()
        })
        _mutableTests.add(test);
        updateStatus()

    }

    fun resetAllTests() {
        _setAllItems(this.tests)
    }

    private fun _setAllItems(tests:List<ITestItem>) {
        for (entry in tests) {
            if (entry is TestGroup) {
                _setAllItems(entry.tests)
            } else if (entry is TestEntry) {
                entry.log.value = ""
                entry.status.value = TestEntryStatus.NotTestedYet
            }
        }
    }
}

data class TestEntry(
    val testcasename: String
    , val testname: String
    , val filepath: String
    , val filename: String
    , val testtext: String

) : ITestItem {
    var requestRun: Boolean = false
    var log: MutableLiveData<String> = MutableLiveData<String>()
    var result: String = ""
    override var status: MutableLiveData<TestEntryStatus> = MutableLiveData(TestEntryStatus.NotTestedYet)

    override val name: String
        get() {
            return "$filename\n$testcasename / $testname"
        }
}

enum class TestEntryStatus(val rep: String) {
    NotTestedYet(""),
    IsTesting("⌛ Testing..."),
    Failed("❌ Failed"),
    Succeeded("✔ OK️✅")
}