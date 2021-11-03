package org.rokist.canlangtest

import android.annotation.SuppressLint
import android.content.Context
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ImageView
import android.widget.TextView
import androidx.databinding.DataBindingUtil
import androidx.fragment.app.Fragment
import androidx.lifecycle.Observer
import androidx.navigation.fragment.findNavController
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import org.rokist.canlangtest.databinding.FragmentTestListBinding
import kotlin.concurrent.thread


class TestListFragment : Fragment() {

    lateinit var binding: FragmentTestListBinding
    val viewModel = SharedModel.instance()

    val testGroup = SharedModel.instance().currentTestItem


    var isTestAutoRan = false

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        binding = DataBindingUtil.inflate(inflater, R.layout.fragment_test_list, container, false)

        context?.toActivity()
            ?.doWithText("abc\uD83D\uDC68\u200D\uD83D\uDC68\u200D\uD83D\uDC67\u200D\uD83D\uDC67{}[]_?><日本語@!test")


        this.binding.root.setBackgroundColor(viewModel.commonBackgroundColor)

        if (testGroup is TestGroup) {
            val myHandler = Handler()

            var lastExecTime: Long = System.currentTimeMillis()

            val runnable = Runnable {
                binding.titleText.text = if (testGroup.status.value == TestEntryStatus.IsTesting) {
                    "▶  " + testGroup.name + "　 "
                } else {
                    "　  " + testGroup.name + "　 "
                }
                MainActivity.getImage(requireContext(), testGroup.status.value)?.also {
                    binding.statusImage.setImageBitmap(it)
                } ?: run {
                    binding.statusImage.setImageDrawable(null)
                }
                this@TestListFragment.binding.mainRecyclerView.adapter?.notifyDataSetChanged();
                lastExecTime = System.currentTimeMillis()
            }


            testGroup.status.observe(viewLifecycleOwner, Observer {
                val diff = System.currentTimeMillis() - lastExecTime
                when {
                    diff < 50 -> {
                        myHandler.removeCallbacks(runnable)
                        //Log.d("aaa", "removed")
                    }
                }

                myHandler.postDelayed(runnable, 50)

            })

            val data = testGroup.tests
            binding.runAllTestsButton.setOnClickListener {
                runAllTests()
            }

            if (testGroup == viewModel.topTestGroup) {
                if (!isTestAutoRan) {
                    // try to find the test case executed last time
                    val lastTimePref = requireActivity().getSharedPreferences(
                        "LastTimeTestEntry",
                        Context.MODE_PRIVATE
                    )
                    val testkey = lastTimePref.getString("testname", null)
                    if (testkey != null && viewModel.testEntryMap.containsKey(testkey)) {
                        val entry = viewModel.testEntryMap[testkey]
                        if (entry != null) {
                            entry.requestRun = true
                            viewModel.currentTestItem = entry

                            findNavController().navigate(R.id.action_first_to_second)
                        }
                    } else {
                        runAllTests()
                    }
                    isTestAutoRan = true
                }
            }

            // reset LastTimeTestEntry
            val lastTimePref = requireActivity().getSharedPreferences(
                "LastTimeTestEntry", Context.MODE_PRIVATE
            )
            lastTimePref.edit().putString("testname", null).apply();


            binding.mainRecyclerView.layoutManager = LinearLayoutManager(requireContext())
            binding.mainRecyclerView.adapter =
                RecyclerAdapter(
                    requireContext(),
                    data,
                    object : OnClickListener {
                        override fun onClickRoot(entry: ITestItem) {
                            Handler(Looper.getMainLooper()).postDelayed({
                                if (entry is TestEntry) {
                                    viewModel.currentTestItem = entry
                                    findNavController().navigate(R.id.action_first_to_second)
                                } else if (entry is TestGroup) {
                                    viewModel.currentTestItem = entry
                                    findNavController().navigate(R.id.action_first_to_first)
                                }
                            }, 10);
                        }
                    })
        }

        /*
        val dividerItemDecoration = DividerItemDecoration(
            binding.mainRecyclerView.context,
            LinearLayoutManager(requireContext()).orientation
        )
        binding.mainRecyclerView.addItemDecoration(dividerItemDecoration)
         */

        return binding.root

    }

    fun runAllTests() {
        if (testGroup is TestGroup) {
            val data = testGroup.tests
            testGroup.resetAllTests()

            thread {
                runAllTests(data)
            }
        }
    }

    private fun runAllTests(data: List<ITestItem>) {

        for (entry in data) {
            if (entry is TestGroup) {
                runAllTests(entry.tests)
            } else if (entry is TestEntry) {
                requireContext().toActivity()
                    ?.runTest(entry.testcasename, entry.testname)
                    ?.let { testToken ->
                        entry.status.postValue(TestEntryStatus.IsTesting)
                        entry.log.postValue("")

                        // Receives logs from the test process
                        for (k in 0..100000) {
                            Thread.sleep(16) // waiting for next log: 60fps

                            val logText = context?.toActivity()?.runTestNext(testToken)
                            if (logText == "[__end_of_test_0];") {
                                entry.status.postValue(TestEntryStatus.Failed)
                                break;
                            } else if (logText == "[__end_of_test_1];") {
                                entry.status.postValue(TestEntryStatus.Succeeded)
                                break;
                            } else {
                                logText?.let {
                                    if (it.isNotEmpty()) {
                                        entry.log.postValue(entry.log.value + it)
                                    }
                                }
                            }
                        }
                    };
            }
        }
    }

}


interface OnClickListener {
    fun onClickRoot(testItem: ITestItem): Unit
}


class ViewHolder(itemView: View) : RecyclerView.ViewHolder(itemView) {
    val root: View = itemView.findViewById(R.id.item_root)
    val button: TextView = itemView.findViewById(R.id.item_textview)
    val titleTextView: TextView = itemView.findViewById(R.id.itemTextView)
    val bodyTextView: TextView = itemView.findViewById(R.id.bodyTextView)
    val statusImage: ImageView = itemView.findViewById(R.id.status_image)
}


class TestGroupViewHolder(itemView: View) : RecyclerView.ViewHolder(itemView) {
    val root: View = itemView.findViewById(R.id.item_root)
    val titleTextView: TextView = itemView.findViewById(R.id.groupTextView)
    val statusImage: ImageView = itemView.findViewById(R.id.status_image)

}

enum class TestItemViewType(val id: Int) {
    TestEntry(0),
    TestGroup(1);
}

class RecyclerAdapter(
    private val context: Context,
    private val data: List<ITestItem>,
    private val clickListener: OnClickListener
) : RecyclerView.Adapter<RecyclerView.ViewHolder>() {

    private val inflater: LayoutInflater = LayoutInflater.from(context)
    val viewModel = SharedModel.instance()


    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): RecyclerView.ViewHolder {
        if (viewType == TestItemViewType.TestEntry.id) {
            val holder = ViewHolder(
                inflater.inflate(
                    R.layout.list_item,
                    parent,
                    false
                )
            )
            holder.bodyTextView.setTextColor(viewModel.secondaryForegroundColor)
            holder.titleTextView.setTextColor(viewModel.secondaryForegroundColor)
            holder.root.setBackgroundResource(R.drawable.list_item_background)
            return holder

        } else {
            val holder = TestGroupViewHolder(
                inflater.inflate(
                    R.layout.list_group_item,
                    parent,
                    false
                )
            )
            holder.titleTextView.setTextColor(viewModel.commonForegroundColor)
            holder.root.setBackgroundResource(R.drawable.list_item_background)
            return holder
        }
    }

    override fun getItemViewType(position: Int): Int {
        val item = data[position]
        return if (item is TestEntry) {
            TestItemViewType.TestEntry.id
        } else {
            TestItemViewType.TestGroup.id
        }
    }

    override fun getItemCount(): Int {
        return data.size
    }


    @SuppressLint("SetTextI18n")
    override fun onBindViewHolder(holder: RecyclerView.ViewHolder, position: Int) {
        val item = data[position];

        if (item is TestEntry) {
            val testHolder = holder as ViewHolder
            testHolder.titleTextView.text = "${item.testcasename}\n${item.filename}"
            testHolder.bodyTextView.text = item.testtext
            testHolder.button.text = "\uD83C\uDF90  "+ //"${item.status.value?.rep}\n" +
                    item.testname

            MainActivity.getImage(context, item.status.value)?.also {
                testHolder.statusImage.setImageBitmap(it)
            } ?: run {
                testHolder.statusImage.setImageDrawable(null)
            }

            testHolder.root.setOnClickListener {
                clickListener.onClickRoot(data[position])
            }


        } else if (item is TestGroup) {
            val groupHolder = holder as TestGroupViewHolder
            groupHolder.titleTextView.text = "\uD83D\uDCE6  ${item.name}" //📁
            //holder.bodyTextView.text = item.name
//            item.status.observe(this,  Observer {
//
//            })

            MainActivity.getImage(context, item.status.value)?.also {
                groupHolder.statusImage.setImageBitmap(it)
            } ?: run {
                groupHolder.statusImage.setImageDrawable(null)
            }


            groupHolder.root.setOnClickListener {
                clickListener.onClickRoot(data[position])
            }
        }
    }


}


