console.log('Gulp starting...')

const path = require('path');
const gulp = require('gulp')
const fs = require('fs')
const util = require('util')
const exec = require('child_process').exec;
const del = require('delete')
const glob = require('glob')


//const username = require('username')
const isWin = /^win/.test(process.platform)
const sep = isWin ? '\\' : '/'

const altCmake = require('./scripts/alt_cmake');

const promise = readyWindowsCommandPrompt()

gulp.task("show", async function (cb) {
    await promise;
    
    const val = await altCmake.dev()
    
    console.info(val)
    cb();
});

// watch and generate a test info json file
gulp.task("watchtest", async function (cb) {
    await promise;

    var gaze_opt = {
        debounceDelay: 1000 // wait 1 sec after the last run
    }

    gulp.watch(
        [
            `./tests/google_tests/*.cc`,
            `./tests/google_tests/*.h`,
            `./tests/google_tests/*.cpp`,
            `./tests/google_tests/*.hh`,
        ],
        gaze_opt,
        async cb => {
            collectTestList()
        }
    )
});


function collectTestList() {

    const testcppfiles = [path.posix.join(".", "tests", "google_tests")];

    const testlist = []

    for (let testdir of testcppfiles) {
        try {
            const files = fs.readdirSync(testdir)

            files.filter(function (file) {
                return /.*\.cpp$/.test(file);
            }).forEach(function (file) {
                let buffer = fs.readFileSync(path.join(testdir, file))
                let text = buffer.toString()

                let currentIndex = 0;
                while (true) {
                    const testIndex = text.indexOf("TEST(", currentIndex)
                    if (testIndex > -1) {
                        const testEndIndex = text.indexOf("ENDTEST", testIndex + 2)
                        const nextTestIndex = text.indexOf("TEST(", testIndex + 2)
                        if (
                            (nextTestIndex == -1 || testEndIndex < nextTestIndex) && testEndIndex > -1) {
                            console.info("========== TEST ==========\n" + text.substring(testIndex, testEndIndex))

                            const testText = text.substring(testIndex, testEndIndex)
                            const indexOfEndParenthes = testText.indexOf(")")

                            const terms = testText.substring(5, indexOfEndParenthes).split(',')

                            if (terms.length > 1) {
                                testlist.push(
                                    {
                                        testcasename: terms[0].trim(),
                                        testname: terms[1].trim(),
                                        filepath: path.posix.join(testdir, file),
                                        filename: file,
                                        body: testText
                                    }
                                )
                            }
                        }
                        currentIndex = testIndex + 2
                    } else {
                        break;
                    }
                }
            });

        } catch (err) {
            console.error(err)
        }
    }

    fs.writeFileSync(
        path.join("AndroidCanLang", "app", "src", "main", "res", "raw", "testlist.json")
        , JSON.stringify(testlist, null, "\t")
    )

    console.info(testlist)
}


function line(str) {
    return str.split('\n').join(' ')
}

function doExec(str, cb) {
    child = exec(line(str), (error, stdout, stderr) => {
        // console.log(error);
        // console.log(stderr || stdout);
        cb(error)
    })

    child.stdout.addListener('data', d => {
        console.log(d)
    })
    child.stderr.addListener('data', d => {
        console.log(d)
    })
}

async function doExecAsync(str) {
    return new Promise((resolve, reject) => {
        child = exec(line(str), (error, stdout, stderr) => {
            resolve(error)
        })

        child.stdout.addListener('data', d => {
            console.log(d)
        })
        child.stderr.addListener('data', d => {
            console.log(d)
        })
    })
}




async function readyWindowsCommandPrompt() {
    if (isWin) {
        await doExecAsync(`chcp 65001`)
    }
}



gulp.task('runTest', cb => {
    runSequence('buildTest', () => {
        let nodeTests = []
        let testList = ''
        glob.sync('unittests/*.ts').forEach(function (filePath) {
            testList += ' ' + filePath.replace('unittests', '')
            nodeTests.push(
                'echo ' +
                filePath +
                ' && node ' +
                (filePath + '')
                    .replace('.ts', '.js')
                    .replace('unittests', 'unittests/out/unittests')
            )
        })
        doExec(nodeTests.join(' && ') + ` && echo ...............OK!`, err => {
            console.info(testList)
            cb(err)
        })
    })
})


gulp.task('wow', function (callback) {

    var password = 'aapa12awjiofe8awo';

    var sha512 = crypto.createHash('sha1');
    sha512.update(password)
    var hash = sha512.digest("hex")
    let ag = parseInt(hash, 16);
    ag = ag % 7;
    console.log(ag)


});


//gulp.task('default', ['typescript'])
//gulp.task('default', ['typescript', 'watch']);