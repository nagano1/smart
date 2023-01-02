/**
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 */

 const exec = require('child_process').exec;
const isWin = /^win/.test(process.platform)
const sep = isWin ? '\\' : '/'
const glob = require('glob')


async function prepareWindowsCommandPrompt() {
    if (isWin) {
        await doExecAsync(`chcp 65001`)
    }
}


async function timeoutAsync(ms) {
    return new Promise((res, rej) => {
        setTimeout(() => {
            res()
        }, ms)
    })
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
        });

        child.stdout.addListener('data', d => {
            console.log(d)
        });
        child.stderr.addListener('data', d => {
            console.log(d)
        });
    })
}

//console.info(cmake.dev());
const srcDir = "./src/";
const testsDir = "../tests";


const globs = ['*.cpp', 'codeNodes/*.cpp']
// "./*.cpp",
// "./codenodes/*.cpp"




module.exports = {
    dev: async function () {
        const github_actions = process.argv.find((v) => v == "--github_actions");
        const m32 = process.argv.find((v) => v == "-m32") ? "-m32" : "";

        const clangPath = isWin && !github_actions ? "C:\\\\Program Files (x86)\\LLVM\\bin\\clang++.exe" : "clang++";

        let cppFileList = "";
        for (const g in globs) {
            glob.sync(srcDir + globs[g]).forEach(function (filePath) {
                cppFileList += " \"" + filePath + "\"";
            });
        }

        //await timeoutAsync(3000);
        const exeRequest = "\"" + clangPath + "\" -std=c++14 -o wow.exe -I src " + m32 + cppFileList;
        console.info(exeRequest);
        await doExecAsync(exeRequest);
    },
    dist: function () {
        return 3;
    }
};