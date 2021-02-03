const path = require('path');
const fs2 = require('fs').promises;
const fs = require('fs');
const gulp = require('gulp');
const del = require('del');
const glob = require('glob')
const os = require('os')
const chokidar = require('chokidar');
const { series } = require('gulp');


const https = require('https');
const exec = require('child_process').exec;

const cpuCount = os.cpus().length;

//#-msse
//#CFLAGS := $(CFLAGS) -g -DNDEBUG
//"-MMD", "-MP", 
let CFLAGS = [
     "-Wall", "-Wextra", "-Winit-self", 
     "-Wno-missing-field-initializers", "-Werror", "-Wno-unused-parameter", 
     "-Wno-unused-variable", "-Wno-unused-function", "-Wno-unused-private-field",
     "-std=c++11", "--target=wasm32-wasi", "--sysroot", "wasi-sysroot",
     "-O3",                                        
];

//let exports_wasm = [];

let exports_wasm = ["__data_end", "__dso_handle" ,"__global_base",
    "__heap_base" ,"__memory_base" ,"__table_base",
    "__wasm_call_ctors" ,"_start" ,"abort" ,"errno"
];

let exports_funcs = ["calling2","calling", "get_memory_for_string", "string_to_int", "free_memory_for_string"];

let ldFlags = [
    "--strip-all",
    //"--export-all",
    "--allow-undefined", "--no-entry", 
    "--lto-O3", "-O3", 
    "--import-memory", "-o", "result.wasm"
];
exports_wasm.map((v)=>ldFlags.push("--export="+v));
exports_funcs.map((v)=>ldFlags.push("--export="+v));

let INCLUDEs   = ["./include", "../src/"];
let SRCDIR    = "./";

let SYSLibDIR = "wasi-sysroot/lib/wasm32-wasi";
let LIBS      = `${SYSLibDIR}/libc.a ${SYSLibDIR}/libc++.a ${SYSLibDIR}/libc++abi.a`;

let clangCompiler = "";
let wasmLinker = "";

let groupASrc = ['wasm.cpp'];
let groupBSrc = ['subwasm.cpp'];
let libCppSrc = ['../src/codenodes/*.cpp'];

let includeGlobs = ["./include/*.hpp", "../src/*.hpp"];
let outTop = "out";



class MainClass {
    constructor() {
        this.resultWasmGenGroup = new ResultWasmGroup("result.wasm");
        this.subWasmGenrGroup = new ResultWasmGroup("sub.wasm");

        this.libDep = new CppCompileGroup("lib", libCppSrc, includeGlobs);
        this.cppDepA = new CppCompileGroup("a", groupASrc, includeGlobs);
        this.cppDepB = new CppCompileGroup("b", groupBSrc, includeGlobs);

        this.resultWasmGenGroup.setDeps([this.libDep, this.cppDepA])
        this.subWasmGenrGroup.setDeps([this.libDep, this.cppDepB]);
    }

    async compileAll() {
        try {
            await Promise.all([
                this.cppDepA.compileAll(),
                this.cppDepB.compileAll(),
                this.libDep.compileAll()
            ]);
        }catch(e) {
            //process.exit(1);
            process.exitCode(3124);
        }
        
    }

    async watch() {
        this.cppDepA.watchFiles();
        this.cppDepB.watchFiles();
        this.libDep.watchFiles();
    }

    async clean() {
        let ls = await doExecAsync("ls -SalR " + outTop);
        console.info("------------------- Files to delete ----------------------");
        console.info(ls);
        console.info("------------------- --------------- ----------------------");
        del(outTop, {force:true});
        console.info("... done");
    }
}


exports["watch"] = exports["w"] = (cb) => {
    new MainClass().watch();
};

exports["build"] = exports["c"] = async (cb) => {
    await new MainClass().compileAll();
};

exports["clean"] = async (cb) => {
    await new MainClass().clean();
};

//exports.default = series();


class DependencyGroup extends Object {
    constructor() {
        super();
        this.fileGlobs = null;
        this.revision = 0;
        this.sumOfTargetRevisions = 0;
        this.resolve = null;
        this.initFinishedPromise();

        this.isProcessing = false;

        this.waitingFileMap = {};
    }

    isFinished() {
        return this.resolve == null;
    }

    async initFinishedPromise() {
        if (this.resolve) {
            console.info("ASSERT ERRor: 102934");
            this.resolve();
            this.resolve = null;
        }

        this.donePromise = new Promise((res,rej)=>{
            this.resolve = res;
        });
    }

    resolvePromise() {
        if (this.resolve) {
            this.resolve();
            this.resolve = null;
        }
    }

    get area() {
        return this.calcArea();
    }

    static eat() {
        return this;
    }
}



class CppCompileGroup extends DependencyGroup {

    constructor(name, fileGlobs, includeGlobs) {
        super();

        this.name = name;
        this.fileGlobs = fileGlobs;
        this.objList = {};
        this.includeGlobs = includeGlobs;
        this.onCompileDone = [];

        this.processingCount = 0;
    }

    async compileAll() {
        await prepareCommands();

        let resolver = null;
        let compilePromise = new Promise((res,rej)=>{
            resolver = res;
        });
        let count = 0;
        let checkDone = () => {
            count--;
            jobQueueManager.notifyJobDone();
            if (count == 0) {
                resolver();
            }
        };

        let func = async (filePath)=> {
            count++;
            await jobQueueManager.waitParallelJobQueue();

            console.info(filePath);
            await this.compileCppFile(filePath);

            checkDone();
        };


        for (let file of this.fileGlobs) {
            glob.sync(file).forEach(func);
        }

        await compilePromise; 

        this.revision ++;
        this.resolvePromise();
        for (let handler of this.onCompileDone) {
            await handler(true);
        }
    }

    getBuildTempPath(filePath) {
       let directory = outTop + "/" + this.name + "/"+ filePath.replace("../", "__pd__/");
       return  directory;

    }

    getAllObjFiles() {
        const list = [];
        const func = (f) => {
            let objFile = this.getBuildTempPath(f).replace(".cpp", ".o");
            list.push(objFile);
        };

        for (let file of this.fileGlobs) {
            glob.sync(file).forEach(func);
        }

        return list;
    }

    async compileCppFile(file) {
        await prepareCommands();

        await fs2.mkdir(path.dirname(this.getBuildTempPath(file)), {recursive:true})
        let objFile = this.getBuildTempPath(file).replace(".cpp", ".o");
        this.objList[objFile] = objFile;
        let compileCmd = [
            clangCompiler, 
            CFLAGS.join(" "), 
            INCLUDEs.map((v)=>`-I${v}`).join(" "),  
            "-o", objFile, 
            "-c", file
        ].join( " " );

        try{
            const stat = await fs2.stat(file);
            console.info(stat.mtimeMs);
        }catch(e) {
            console.info(e);
        }

        //console.info(compileCmd);
        console.time(file);
        let output = await doExecAsync(compileCmd);
        console.timeEnd(file);
        //console.info(`output = ${output}`);
    }

    async watchFiles() {
        await this.compileAll();

        let targetFiles = this.fileGlobs.concat(this.includeGlobs)

        console.info("watch target files:");
        console.info(targetFiles);

        // all, add, addDir, change, unlink, unlinkDir, ready, raw, error
        chokidar.watch(targetFiles).on('all', async (event, file) => {
            console.info(`watch event: ${event}`);
            if (event == "change") {
                if (file.indexOf(".hpp") > -1) {
                    await this.initFinishedPromise();
                    await this.compileAll();
                } else {
                    console.info(`changed file: ${file}`);

                    let resolveFilePromise = null; 
                    if (this.waitingFileMap[file]) {
                        let fileItem = this.waitingFileMap[file];

                        if (fileItem.promise) {
                            fileItem.runningIndex++;
                            let thisIndex = fileItem.runningIndex;

                            await fileItem.promise;

                            if (thisIndex != fileItem.runningIndex) {
                                /*
                                 * runningIndex was modified, which means 
                                 * another function is trying to compile
                                 */
                                return;
                            }
                        } 
                    } else {
                        this.waitingFileMap[file] = {
                            runningIndex: 0
                        };
                    }

                    this.waitingFileMap[file].promise = new Promise((ok,ng)=>{
                        resolveFilePromise = ok; 
                    });


                    if (this.processingCount == 0) {
                        await this.initFinishedPromise();
                    }

                    this.processingCount++;

                    await this.compileCppFile(file);
                    this.revision++;

                    if (resolveFilePromise) {
                        resolveFilePromise();
                    }

                    this.processingCount--;
                    if (this.processingCount == 0) {
                        this.resolvePromise();
                        for (let handler of this.onCompileDone) {
                            await handler(true);
                        }
                    }
                }
            }
        });
    }

}


class ResultWasmGroup extends DependencyGroup {
    constructor(outputFileName) {
        super();
        this.outputFileName = outputFileName;
        this.prevSum = "-0"
        this.depList = null;
    }


    setDeps(depList) {
        this.depList = depList;

        for (let dep of depList) {
            dep.onCompileDone.push(async (last) => {
                for (let d of depList) {
                    if(d.isFinished() == false) {
                        return true;
                    }
                }
                let promises = depList.map((dep)=>dep.donePromise);
                await Promise.all(promises);

                let revisionRep = depList.map((dep)=>{return ""+dep.revision}).join("");
                if (this.prevSum != revisionRep) {
                    this.prevSum = revisionRep;
                    this.doLink();
                }
            });
        }
    }

    async doLink(globs) {
        let srcFiles = [];

        for (let d of this.depList) {
            for (let n in d.objList) {
                srcFiles.push(n);
            }
        }

        console.info(srcFiles);
        let linkCmd = [
            wasmLinker, 
            ldFlags.join(" "), 
            "-o " +outTop + "/" +this.outputFileName, 
            srcFiles.join(" "),
            LIBS
        ].join( " " );

        //console.info(linkCmd);
        console.time(this.outputFileName)
        let output = await doExecAsync(linkCmd);
        console.timeEnd(this.outputFileName)

        let ls = await doExecAsync("ls -Sal " + outTop);
        console.info("link done: " + this.outputFileName);
        console.info("ls: " + ls);
    }
}

















let preparePromise = null;
async function prepareCommands() {
    if (preparePromise) {
        return preparePromise;
    }

    preparePromise = new Promise(async (resolve, reject)=>{
        let compiler = await doExecAsync("which clang++-10", true);
        if (compiler) {
            clangCompiler = compiler;
        } else if (compiler = await doExecAsync("which clang++-9", true)) {
            clangCompiler = compiler;
        } else if (compiler = await doExecAsync("clang++ --help", true)) {
            clangCompiler = "clang++"; // this is for windows on github actions
        }


        let linker = await doExecAsync("which wasm-ld-10", true);
        if (linker) {
            wasmLinker = linker;
        } else if (linker = await doExecAsync("which wasm-ld-9", true)) {
            wasmLinker = linker;
        } else if (linker = await doExecAsync("wasm-ld --help", true)) {
            wasmLinker = "wasm-ld";
        }

        clangCompiler = clangCompiler.trim();
        wasmLinker = wasmLinker.trim();

        console.info(`Compiler: \t ${clangCompiler}`);
        console.info(`Wasm Linker: \t ${wasmLinker}`);

        resolve();
    });
    return preparePromise;
}

const MAX_PARALLEL = cpuCount;
const jobQueueManager = {
    execQueue: [],
    executingCount: 0,
    intervalId: null,

    /**
     * wait until executingCount is under 4
     */
    notifyJobDone: function () {
        this.executingCount--;
        this.tryNext();
    },

    tryNext: function () {
        for (let i = 0; i < MAX_PARALLEL; i++) {
            if(this.executingCount < MAX_PARALLEL) {
                let res = this.execQueue.shift();
                if (res){
                    res();
                    this.executingCount++;
                }
            }
        }
    },

    waitParallelJobQueue: async function() {
        let promise = new Promise((res, rej) => {
            this.execQueue.push(res);
        });

        if (this.intervalId == null) {
            //this.intervalId = setInterval(this.tryNext, 3000);
        }

        this.tryNext();

        return promise;
    },


};





















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

async function doExecAsync(str, hideError=false) {
    return new Promise((resolve, reject) => {
        child = exec(line(str), (error, stdout, stderr) => {
            if (error) {
                console.log(stderr)
                if (hideError){ 
                    resolve(null)
                } else {
                    reject(stderr)
                }
            } else {
                resolve(stdout)
            }
        })

        child.stdout.addListener('data', d => {
            //console.log(d)
        })
        child.stderr.addListener('data', d => {
            console.log(d)
        })
    })
}


async function download(url, filename) {
    return new Promise((resolve,rej)=>{
        var outFile = fs.createWriteStream(filename);
        const req = https.get(url, function (res) {
                console.info(res.statusCode);
            res.pipe(outFile);
            res.on('end', function () {
                outFile.close();
                resolve();
            });
        });

        req.on('error', function (err) {
            console.log('Error: ', err); return;
            rej();
        });
    });
}

