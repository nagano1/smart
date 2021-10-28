const puppeteer = require('puppeteer');

function slp(milliseconds) {
    return new Promise((res, rej) => {
        setTimeout(() => {
            res();
        }, milliseconds);
    });
}

(async () => {
    console.info(process.argv);

    const os = process.argv[2];
    const URL = `http://okcom.180r.com/files/${os}/test.html`;
    console.info(URL);

    
    const browser = await puppeteer.launch({});
    const page = await browser.newPage();

    let okOnce = false

    page.on('console',
        async msg => {
            console.log('PAGE LOG:', msg.text());
            try {
                const json = JSON.parse(msg.text());
                if (json.isTestResult) {
                    if (json.ok) {
                        console.log("ok:");
                        okOnce = true;
                    } else {
                        await process.exit(240);
                    }
                }
            } catch(e) {
                //console.log(e)
            }
        }
    );

    await page.goto(URL);


    // Get the "viewport" of the page, as reported by the page.
    const dimensions = await page.evaluate(() => {
        return {
            width: document.documentElement.clientWidth,
            height: document.documentElement.clientHeight,
            title: document.title,
            deviceScaleFactor: window.devicePixelRatio
        };
    });

    // wait for loading web assembly
    for (let t = 0; t < 10; t++) {
        await slp(2000);
        if (okOnce) {
            break;
        }
    }

    if (!okOnce) {
        console.error("okOnce error");
        await process.exit(118);
    }
    

    console.log('Dimensions:', dimensions);
    console.log('title:', dimensions.title);

    await browser.close();
})();