console.log('Gulp starting...')

const path = require('path');
const gulp = require('gulp')
const fs = require('fs')
const util = require('util')
const exec = require('child_process').exec, child
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
        debounceDelay: 1000 // wait 4 sec after the last run
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
                            console.info("========== TEST :-------------------\n" + text.substring(testIndex, testEndIndex))

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

const WebPack = isWin
    ? 'node_modules\\.bin\\webpack'
    : 'node_modules/.bin/webpack'


gulp.task('watch_native', async cb => {
    runSequence('watch_main_entry', 'watch_parser_addon', cb)
})


 



async function aRe() {
    return new Promise((res, rej) => {
        // runSequence(['buildaddon'], () => {
        setTimeout(() => {
            res()
        }, 4000)
        // });
    })
}




async function embedJsIntoCC(
    productMode,
    jsPath,
    srcCCDir,
    targetDir,
    originalCCFiles
) { }

async function readyWindowsCommandPrompt() {
    if (isWin) {
        await doExecAsync(`chcp 65001`)
    }
}



gulp.task('buildLayout', cb => {
    doExec(`${WebPack} --config layoutTest/webpack.config.js`, cb)
})

gulp.task('watchLayout', cb => {
    doExec(`${WebPack} -w --config layoutTest/webpack.config.js`, cb)
})

gulp.task('distLayout', cb => {
    doExec(`cd layoutTest/dist && npm run dist`, cb)
})



// Unit Tests

gulp.task('buildTest', cb => {
    doExec(`tsc -p unittests --outDir unittests/out`, cb)
})

// tsc --target "es5" --lib  "dom,es2015,es6" unittests/stream_test.ts --outDir unittests/out  && ./node_modules/.bin/ts-node unittests/out/unittests/stream_test.js

// ./node_modules/.bin/ts-node unittests/stream_test.ts

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

// Virtual List Sample
let webpack_virtual_list_js = 'samples/webpack.virtual_list.js'
gulp.task('buildVirtualList', cb => {
    var args = process.argv.slice(2)

    // if (args[0] === "--env" && args[1] === "production");
    console.log(args)
    doExec(`${WebPack} --config ${webpack_virtual_list_js} `, cb)
})

gulp.task('watchVirtualList', cb => {
    doExec(`${WebPack} --watch --config ${webpack_virtual_list_js} `, cb)
})
gulp.task('runVirtualList', cb => {
    doExec(`${nw_exe} samples/virtual_list_test/virtual_list_debug`, cb)
})

var targetTS = ['./src/*.ts']

gulp.task('watch', function () {
    gulp.watch(targetTS, ['watch_task'])
})

gulp.task('watch_task', function (callback) {
    // return runSequence( 'clearTerminal', 'typescript', callback);
    return runSequence('typescript', callback)
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
gulp.task('js_test', function (callback) {
    // return runSequence( 'clearTerminal', 'typescript', callback);
    //return runSequence('typescript', callback)

    function customAsync(val) {
        if (val < 50) {
            return new Promise((res, rej) => {

            });
        }
        return 54
    }

    var g = customAsync(46);
    console.info(g)
})


//gulp.task('default', ['typescript'])
// gulp.task('default', ['typescript', 'watch']);

gulp.task('typescript', function (cb) {
    console.log('start compiling with typescript')
    gulp
        .src(targetTS)
        // .pipe(plumber())
        .pipe(
            ts({
                target: 'ES6',
                noImplicitAny: true,
                strictNullChecks: true
            })
        )
        .pipe(concat('all.js'))
        // .pipe(uglify())
        // .pipe(babel({
        // presets: ['es2015']
        // }))
        // .pipe(rename(function(path) { path.dirname = path.dirname.replace('ts', 'js') }))
        // .pipe(uglify())

        .pipe(gulp.dest('./dist/'))
        .on('end', cb)
    /* .on('end', function() {
            console.log("compile end");
            child = exec('uglifyjs dist/all_.js -o dist/all.js',
            function (error, stdout, stderr) {
                console.log("test node finished");
                console.log('\n' + stdout);
  
                if (stderr) {
                    console.log('stderr: ' + stderr);
                }
  
                if (error !== null) {
                    console.log('exec error: ' + error);
                }
                //process.exit();
                cb();
            });
       }); */
})
