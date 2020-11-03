package org.rokist.canlangtest

import android.content.Context
import android.graphics.Color
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.text.Spannable
import android.text.SpannableString
import android.text.style.ForegroundColorSpan
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import androidx.databinding.DataBindingUtil
import androidx.fragment.app.Fragment
import androidx.fragment.app.FragmentManager
import androidx.fragment.app.FragmentPagerAdapter
import androidx.lifecycle.Observer
import org.rokist.canlangtest.databinding.FragmentTabCodeBinding
import org.rokist.canlangtest.databinding.FragmentTabLogBinding
import org.rokist.canlangtest.databinding.FragmentTestEntryBinding
import kotlin.concurrent.thread


class TestEntryFragment : Fragment() {
    private lateinit var adapter: TabAdapter
    lateinit var binding: FragmentTestEntryBinding
    val sharedData = SharedModel.instance()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        requireActivity().setTitle("test")
    }

    override fun onAttach(context: Context) {
        super.onAttach(context)
        adapter = TabAdapter(childFragmentManager, requireContext())
    }

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        binding = DataBindingUtil.inflate(inflater, R.layout.fragment_test_entry, container, false)

        binding.pager.setBackgroundColor(sharedData.commonBackgroundColor)

        //binding.tabLayout.setSelectedTabIndicatorColor(sharedData.commonForegroundColor);
        binding.tabLayout.setTabTextColors(
            sharedData.tabHiddenTextColor,
            sharedData.commonForegroundColor
        );

        val test = sharedData.currentTestItem
        if (test is TestEntry) {
            // save lasttime testentry

            val lastTimePref =
                requireActivity().getSharedPreferences("LastTimeTestEntry", Context.MODE_PRIVATE)
            lastTimePref.edit().putString("testname", test.name).apply();


            binding.titleText.text = test.name
            test.status.observe(viewLifecycleOwner, Observer {
                MainActivity.getImage(requireContext(), test.status.value)?.also {
                    binding.statusImage.setImageBitmap(it)
                } ?: run {
                    binding.statusImage.setImageDrawable(null)
                }
            })
        }

        binding.pager.adapter = adapter
        binding.tabLayout.setupWithViewPager(binding.pager)

        return binding.root
    }


}


class Tab02Fragment : Fragment() {

    private val sharedData = SharedModel.instance()
    lateinit var binding: FragmentTabLogBinding

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        binding = DataBindingUtil.inflate(inflater, R.layout.fragment_tab_log, container, false)

        setup()
        return binding.root
    }



    private fun setup() {

        val test = sharedData.currentTestItem

        binding.textView.setBackgroundColor(sharedData.commonBackgroundColor)
        binding.textView.setTextColor(sharedData.commonForegroundColor)

        if (test is TestEntry) {
            test.log.observe(viewLifecycleOwner, Observer {
                binding.textView.setText(it)
            })

            thread {
                //val spannable: Spannable = generateSpannable(test)

                Handler(Looper.getMainLooper()).post {
                    binding.textView.text =
                        test.log.value // spannable, TextView.BufferType.SPANNABLE)
                }
            }

            if (test.requestRun) {
                thread {
                        test.log.postValue("Preparing...\n")
                            Thread.sleep(400)
                            Handler(Looper.getMainLooper()).post {
                                runTest()
                            }
                }
                test.requestRun = false
            }

            binding.runTestButton.setOnClickListener {
                runTest()
            }

        } else {
            binding.textView.text = "error"
        }

    }

    private fun runTest() {
        val test = sharedData.currentTestItem

        if (test is TestEntry) {
            thread {
                requireContext().toActivity()
                    ?.runTest(test.testcasename, test.testname)
                    ?.let {
                        test.status.postValue(TestEntryStatus.IsTesting)
                        test.log.postValue("Started\n")


                        //while (true) {
                        for (k in 0..100000) {

                            Thread.sleep(16);

                            val logText = context?.toActivity()?.runTestNext(it)

                            if (logText == "[__end_of_test_0];") {
                                Log.d("aaa jfowe", "finished log = $logText")
                                test.status.postValue(TestEntryStatus.Failed)

                                break
                            } else if (logText == "[__end_of_test_1];") {
                                Log.d("aaa jfowe", "finished log = $logText")
                                test.status.postValue(TestEntryStatus.Succeeded)
                                break
                            } else {
                                logText?.let {
                                    if (it.length > 0) {
                                        test.log.postValue(test.log.value + it)

                                        Log.d(
                                            "aaa jfowe",
                                            "log = [$it]"
                                        )
                                    }
                                }

                            }
                        }
                    }
            }
        }


    }
}


class TabAdapter(fm: FragmentManager, private val context: Context)
    : FragmentPagerAdapter(fm,FragmentPagerAdapter.BEHAVIOR_RESUME_ONLY_CURRENT_FRAGMENT) {
    override fun getItem(position: Int): Fragment {
        return when (position) {
            0 -> Tab02Fragment()
            else -> TabCodeFragment()
        }
    }

    override fun getPageTitle(position: Int): CharSequence? {
        return when (position) {
            0 -> "Log"
            else -> "Code"
        }
    }

    override fun getCount(): Int {
        return 2
    }

}


class TabCodeFragment : Fragment() {
    private val sharedData = SharedModel.instance()
    lateinit var binding: FragmentTabCodeBinding


    var setupped = false
    override fun onResume() {
        super.onResume()

        if (!setupped) {
            setupped = true
            setup()
        }
    }


    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        binding = DataBindingUtil.inflate(inflater, R.layout.fragment_tab_code, container, false)
        return binding.root
    }

    private fun colorDict(): MutableMap<String, Int> {
        val colorDict = mutableMapOf<String, Int>()

        val color1 = Color.parseColor("#FFFF00")
        colorDict["int"] = color1
        colorDict["long"] = color1
        colorDict["unsigned"] = color1
        colorDict["char"] = color1
        colorDict["float"] = color1
        colorDict["double"] = color1
        colorDict["string"] = color1
        colorDict["class"] = color1
        colorDict["for"] = color1
        colorDict["return"] = color1
        colorDict["continue"] = color1
        colorDict["break"] = color1
        colorDict["std"] = color1
        colorDict["while"] = color1
        colorDict["false"] = color1
        colorDict["if"] = color1
        colorDict["else"] = color1
        colorDict["true"] = Color.parseColor("#FFFF00")
        colorDict["false"] = Color.parseColor("#FFFF00")
        colorDict["new"] = Color.parseColor("#FFFF00")
        colorDict["auto"] = Color.parseColor("#FFFF00")
        colorDict["mauto"] = Color.parseColor("#FFFF00")
        colorDict["mut"] = Color.parseColor("#FFFF00")
        colorDict["muta"] = Color.parseColor("#FFFF00")


        colorDict["EXPECT_EQ"] = Color.parseColor("#9857C6")
        colorDict["EXPECT_LT"] = Color.parseColor("#9857C6")
        colorDict["EXPECT_GT"] = Color.parseColor("#9857C6")
        colorDict["TEST"] = Color.parseColor("#9857C6")
        colorDict["GLOG"] = Color.parseColor("#9857C6")

        return colorDict

    }

    private fun setup() {
        val test = sharedData.currentTestItem

        binding.textView.setBackgroundColor(sharedData.commonBackgroundColor)
        binding.textView.setTextColor(sharedData.commonForegroundColor)

        if (test is TestEntry) {
            thread {
                val startTime = System.currentTimeMillis()
                val spannable: Spannable = generateSpannable(test)

                val delayTime = (400 - (System.currentTimeMillis() - startTime)).also {
                    if (it < 1) 1 else it
                }
                Handler(Looper.getMainLooper()).postDelayed({
                    Handler(Looper.getMainLooper()).postDelayed({
                        binding.progressBar.visibility = View.INVISIBLE
                    }, 200)
                    binding.textView.setText(spannable, TextView.BufferType.SPANNABLE)

                }, delayTime)
            }

        } else {
            binding.progressBar.visibility = View.INVISIBLE
            binding.textView.text = "error"
        }

    }

    private fun generateSpannable(test: TestEntry): Spannable {
        // use \u00A0 to wrap by letters
        val spannable: Spannable = SpannableString(test.testtext.replace(" ", "\u00A0"))

        val lines = test.testtext.split("\n")
        var masterIndex = 0
        val colorDict = colorDict()
        for (line in lines) {
            var currentIndex = masterIndex

            val terms = line.split(
                "(", ")", " ", "*", "+", "-", "%", "!", "[", "]", ";", ":", ",",
                ".", "&", "{", "}", "<", ">", "/", "\"", "'", "=", "\t",
                ignoreCase = false
            )

            // no need to put much effort to print beautifully, I guess
            for (term in terms) {
                val color = colorDict[term] ?: Color.parseColor("#DDDDDD")
                spannable.setSpan(
                    ForegroundColorSpan(color),
                    currentIndex,
                    currentIndex + term.length,
                    Spannable.SPAN_EXCLUSIVE_EXCLUSIVE
                )

                if (currentIndex + term.length + 1 < test.testtext.length) {
                    spannable.setSpan(
                        ForegroundColorSpan(Color.CYAN),//Color.parseColor("#ddff55")),
                        currentIndex + term.length,
                        currentIndex + term.length + 1,
                        Spannable.SPAN_EXCLUSIVE_EXCLUSIVE
                    )
                }
                currentIndex += term.length + 1
            }


            // simple double quotes
            val countOfQuote = line.count { it == '"' }
            if (countOfQuote >= 2 && countOfQuote % 2 == 0) {
                val words = line.split("\"");
                var startIndex = 0
                var odd = false
                for (word in words) {
                    if (odd) {
                        try {
                            spannable.setSpan(
                                ForegroundColorSpan(Color.GREEN),
                                masterIndex + startIndex - 1,
                                masterIndex + startIndex + word.length + 1,
                                Spannable.SPAN_EXCLUSIVE_EXCLUSIVE
                            )
                        } catch (ex: Exception) {
                            Log.d("ex", ex.message ?: "error")
                        }
                    }
                    startIndex += word.length + 1
                    odd = !odd
                }
            }


            // find one line comment
            val indexOfComment = line.indexOf("//")
            if (indexOfComment > -1) {
                spannable.setSpan(
                    ForegroundColorSpan(Color.GRAY),
                    masterIndex + indexOfComment,
                    masterIndex + line.length,
                    Spannable.SPAN_EXCLUSIVE_EXCLUSIVE
                )
            }

            masterIndex += line.length + 1


        }
        return spannable
    }
}