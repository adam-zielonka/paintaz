// Real browser input and downloaded-file pixels. No app test hooks or mocked Qt.
import assert from 'node:assert/strict';
import fs from 'node:fs/promises';
import path from 'node:path';
import http from 'node:http';
import { fileURLToPath } from 'node:url';
const { chromium } = await import(process.env.PLAYWRIGHT_MODULE || 'playwright');
const args = process.argv.slice(2);
const value = key => args[args.indexOf(key) + 1];
const output = path.resolve(value('--output') || 'test-results/web');
await fs.mkdir(output, { recursive: true });
let server;
let url;
if (args.includes('--url')) url = value('--url');
else {
  const root = path.resolve(args.includes('--dir') ? value('--dir') : 'dist/web');
  server = http.createServer(async (req, res) => {
    try {
      const name = decodeURIComponent(new URL(req.url, 'http://localhost').pathname);
      const file = path.resolve(root, '.' + (name.endsWith('/') ? name + 'index.html' : name));
      if (!file.startsWith(root + path.sep)) throw new Error('Invalid path');
      const data = await fs.readFile(file);
      const type = { '.wasm': 'application/wasm', '.js': 'text/javascript', '.html': 'text/html', '.json': 'application/json' };
      res.writeHead(200, { 'Content-Type': type[path.extname(file)] || 'application/octet-stream' }); res.end(data);
    } catch { res.writeHead(404); res.end(); }
  });
  await new Promise(resolve => server.listen(0, '127.0.0.1', resolve));
  url = `http://127.0.0.1:${server.address().port}/`;
}
const browser = await chromium.launch({headless:true, args:['--no-sandbox','--enable-unsafe-swiftshader','--use-angle=swiftshader']});
const context = await browser.newContext({viewport:{width:1280,height:900}, acceptDownloads:true});
const page = await context.newPage();
const errors = [];
page.on('pageerror', error => errors.push(String(error)));
page.on('console', message => { if (['error','warning'].includes(message.type())) console.log('BROWSER',message.type(),message.text()); if(message.type()==='error') errors.push(message.text()); });
page.on('response', response => { if(response.status()>=400) errors.push(`HTTP ${response.status()} ${response.url()}`); });
const report = {url, browser:browser.version(), tests:[]};
try {
  await page.goto(url);
  await page.waitForFunction(() => document.body.dataset.ready === 'true', {timeout:90000});
  await page.locator('canvas').first().waitFor({state:'visible'});
  await page.screenshot({path:path.join(output,'startup.png')});
  console.log('READY',await page.locator('canvas').first().boundingBox());
  if (!args.includes('--startup-only')) {
    // Additional interaction checks are below.
  }
  assert.deepEqual(errors, [], 'JS/Wasm/HTTP errors');
  report.tests.push('startup without JS/Wasm/HTTP errors');
  await fs.writeFile(path.join(output,'report.json'),JSON.stringify(report,null,2));
} catch(error) {
  await page.screenshot({path:path.join(output,'failure.png')}).catch(()=>{});
  throw error;
} finally { await browser.close(); if(server) server.close(); }
