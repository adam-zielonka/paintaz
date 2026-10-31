// Real browser input and downloaded-file pixels. No app test hooks or mocked Qt.
import assert from 'node:assert/strict';
import fs from 'node:fs/promises';
import path from 'node:path';
import http from 'node:http';
const { chromium } = await import(process.env.PLAYWRIGHT_MODULE || 'playwright');
const args = process.argv.slice(2);
const value = key => args.includes(key) ? args[args.indexOf(key) + 1] : undefined;
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
      const stats = await fs.stat(file);
      const data = req.method === 'HEAD' ? undefined : await fs.readFile(file);
      const type = { '.wasm': 'application/wasm', '.js': 'text/javascript', '.html': 'text/html', '.json': 'application/json' };
      res.writeHead(200, { 'Content-Type': type[path.extname(file)] || 'application/octet-stream', 'Content-Length':stats.size }); res.end(data);
    } catch { res.writeHead(404); res.end(); }
  });
  await new Promise(resolve => server.listen(0, '127.0.0.1', resolve));
  url = `http://127.0.0.1:${server.address().port}/`;
}
const browser = await chromium.launch({headless:true, args:['--no-sandbox','--enable-unsafe-swiftshader','--use-angle=swiftshader']});
const context = await browser.newContext({viewport:{width:1280,height:900}, acceptDownloads:true,hasTouch:true});
const page = await context.newPage();
const errors = [];
page.on('pageerror', error => errors.push(String(error)));
page.on('console', message => { if (['error','warning'].includes(message.type())) console.log('BROWSER',message.type(),message.text()); if(message.type()==='error') errors.push(message.text()); });
page.on('response', response => { if(response.status()>=400) errors.push(`HTTP ${response.status()} ${response.url()}`); });
const report = {url, browser:browser.version(), tests:[]};
const tick = () => page.waitForTimeout(180);
const shortcut = async key => { await page.keyboard.press(key); await tick(); };
const decode = async (file,sample) => page.evaluate(async ({encoded,sample}) => {
  const image = new Image(); image.src = 'data:image/png;base64,' + encoded;
  await image.decode();
  const canvas = document.createElement('canvas'); canvas.width=image.width; canvas.height=image.height;
  const ctx=canvas.getContext('2d'); ctx.drawImage(image,0,0);
  if(sample) return Array.from(ctx.getImageData(sample.x,sample.y,1,1).data).slice(0,3);
  return {width:image.width,height:image.height,pixels:Array.from(ctx.getImageData(0,0,image.width,image.height).data)};
}, {encoded:(await fs.readFile(file)).toString('base64'),sample});
const pixel = (image,x,y) => image.pixels.slice((y*image.width+x)*4,(y*image.width+x)*4+3);
let downloadIndex = 0;
const download = async (key='Control+s') => {
  const event = page.waitForEvent('download', {timeout:15000});
  await shortcut(key);
  const item = await event;
  assert.match(item.suggestedFilename(), /\.png$/i);
  const file=path.join(output,`download-${++downloadIndex}.png`);
  await item.saveAs(file);
  return {file,...await decode(file)};
};
const open = async (name,buffer,mimeType) => {
  const chooser=page.waitForEvent('filechooser');
  const surface=await page.locator('canvas').first().boundingBox();
  await page.mouse.click(62,surface.y+43);
  await (await chooser).setFiles({name,buffer,mimeType});
  await tick();
};
const newImage = async (width,height,accept=true,pointer=false) => {
  if(pointer) {
    const surface=await page.locator('canvas').first().boundingBox();
    await page.mouse.click(29,surface.y+43);await tick();
  } else await shortcut('Control+n');
  await page.locator('canvas').nth(1).waitFor({state:'visible'});
  const dialog=await page.locator('canvas').nth(1).boundingBox();
  await page.mouse.click(dialog.x+150,dialog.y+25);
  await page.keyboard.press('Control+a'); await page.keyboard.type(String(width));
  await page.keyboard.press('Tab'); await page.keyboard.press('Control+a'); await page.keyboard.type(String(height));
  await shortcut(accept ? 'Enter' : 'Escape');
};
const drag = async (x1,y1,x2,y2) => {
  await page.mouse.move(x1,y1); await page.mouse.down();
  await page.mouse.move(x2,y2,{steps:12}); await page.mouse.up(); await tick();
};
const bmpFixture = () => {
  const width=64,height=48,stride=width*3;
  const data=Buffer.alloc(54+stride*height);
  data.write('BM'); data.writeUInt32LE(data.length,2); data.writeUInt32LE(54,10);
  data.writeUInt32LE(40,14); data.writeInt32LE(width,18); data.writeInt32LE(height,22);
  data.writeUInt16LE(1,26); data.writeUInt16LE(24,28); data.writeUInt32LE(stride*height,34);
  for(let y=0;y<height;y++) for(let x=0;x<width;x++) {
    const rgb=x<16 && y<16 ? [32,64,240] : [240,32,64];
    const i=54+(height-1-y)*stride+x*3; data[i]=rgb[2];data[i+1]=rgb[1];data[i+2]=rgb[0];
  }
  return data;
};
const tiffFixture = () => {
  // Valid baseline little-endian grayscale TIFF, 2x2, uncompressed single strip.
  const tags=[[256,4,2],[257,4,2],[258,3,8],[259,3,1],[262,3,1],[273,4,122],[277,3,1],[278,4,2],[279,4,4]];
  const data=Buffer.alloc(126); data.write('II');data.writeUInt16LE(42,2);data.writeUInt32LE(8,4);data.writeUInt16LE(tags.length,8);
  tags.forEach(([tag,type,n],i)=>{const at=10+i*12;data.writeUInt16LE(tag,at);data.writeUInt16LE(type,at+2);data.writeUInt32LE(1,at+4);data.writeUInt32LE(n,at+8);});
  data.set([0,255,128,64],122);return data;
};
const fixture = async (format='png',width=64,height=48) => Buffer.from(await page.evaluate(({format,width,height})=>{
  const canvas=document.createElement('canvas'); canvas.width=width; canvas.height=height;
  const ctx=canvas.getContext('2d'); ctx.fillStyle='#f02040'; ctx.fillRect(0,0,width,height);
  ctx.fillStyle='#2040f0'; ctx.fillRect(0,0,16,16);
  return canvas.toDataURL('image/'+format,1).split(',')[1];
},{format,width,height}),'base64');
try {
  const metadata=await page.request.get(new URL('build-info.json',url).href);
  assert.equal(metadata.status(),200);
  const build=await metadata.json();report.commit=build.commit;
  if(args.includes('--expect-sha')) assert.equal(build.commit,value('--expect-sha'));
  for(const name of [...Object.keys(build.files),'licenses/qtbase/LICENSES/GPL-3.0-only.txt']) {
    const response=await page.request.head(new URL(name,url).href);
    assert.equal(response.status(),200,`available distribution file: ${name}`);
  }
  report.tests.push('build identity, runtime/source/license availability');
  await page.goto(url);
  await page.waitForFunction(() => document.body.dataset.ready === 'true', {timeout:90000});
  await page.locator('canvas').first().waitFor({state:'visible'});
  await page.screenshot({path:path.join(output,'startup.png')});
  console.log('READY',await page.locator('canvas').first().boundingBox());
  if (!args.includes('--startup-only')) {
    const box=await page.locator('canvas').first().boundingBox();
    const origin={x:box.x+1,y:box.y+139};
    await newImage(64,48);
    const blank = await download();
    await shortcut('1');
    await page.mouse.click(origin.x+20,origin.y+20);
    await shortcut('Control+z');
    assert.deepEqual(errors, [], 'stationary click followed by Undo must not trap');
    assert.deepEqual((await download()).pixels,blank.pixels);
    await shortcut('2');
    await drag(origin.x+5,origin.y+8,origin.x+50,origin.y+8);
    const beforeUndo = await download();
    assert.deepEqual(pixel(beforeUndo,20,8),[0,0,0]);
    await shortcut('Control+z');
    assert.deepEqual((await download()).pixels,blank.pixels);
    await shortcut('1');
    await page.mouse.click(origin.x+20,origin.y+20);
    await shortcut('Control+y');
    assert.deepEqual((await download()).pixels,beforeUndo.pixels,
      'stationary click after Undo must preserve Redo');
    await shortcut('Control+z');
    assert.deepEqual((await download()).pixels,blank.pixels);
    report.tests.push('stationary click creates no undo step and preserves redo pixels');
    await newImage(64,48);
    let saved=await download();
    assert.equal(saved.width,64); assert.equal(saved.height,48);
    assert.deepEqual(pixel(saved,20,5),[255,255,255]);
    await shortcut('2');
    await drag(origin.x+5,origin.y+5,origin.x+50,origin.y+5);
    saved=await download();
    assert.deepEqual(pixel(saved,20,5),[0,0,0]);
    const drawing=saved.pixels;
    await shortcut('Control+z'); assert.deepEqual(pixel(await download(),20,5),[255,255,255]);
    await shortcut('Control+y'); assert.deepEqual((await download()).pixels,drawing);
    await newImage(32,24,false); assert.deepEqual((await download()).pixels,drawing);
    await newImage(0,48); await shortcut('Escape'); assert.deepEqual((await download()).pixels,drawing);
    report.tests.push('new accept/cancel/invalid size, line pixels, undo/redo');
    const color = async (button, hex, accept=true) => {
      await page.mouse.click(button,box.y+43); await tick();
      await page.mouse.click(790,575); await page.keyboard.press('Control+a'); await page.keyboard.type(hex);
      await page.screenshot({path:path.join(output,`${button===530?'pen':'brush'}-${accept?'accept':'cancel'}.png`)});
      await page.mouse.click(accept?779:865,614); await tick();
    };
    await color(530,'#e02030');
    await color(530,'#00ffff',false);
    await shortcut('2'); await drag(origin.x+5,origin.y+10,origin.x+50,origin.y+10);
    assert.deepEqual(pixel(await download(),20,10),[224,32,48]);
    await color(590,'#2060e0');
    await color(590,'#ffff00',false);
    await page.mouse.click(234,box.y+118); // Brush Full
    await shortcut('3'); await drag(origin.x+10,origin.y+20,origin.x+50,origin.y+40);
    saved=await download('Control+Shift+s');
    assert.deepEqual(pixel(saved,20,30),[32,96,224]);
    assert.deepEqual(pixel(saved,10,30),[224,32,48]);
    await page.screenshot({path:path.join(output,'drawing.png')});
    report.tests.push('pen and brush accept/cancel, filled rectangle pixels, Save As download');

    const png=await fixture();
    await open('synthetic.png',png,'image/png');
    saved=await download();
    assert.equal(saved.width,64);assert.equal(saved.height,48);
    assert.deepEqual(pixel(saved,5,5),[32,64,240]);
    assert.deepEqual(pixel(saved,30,30),[240,32,64]);
    await open('roundtrip.png',await fs.readFile(saved.file),'image/png');
    assert.deepEqual((await download()).pixels,saved.pixels);
    await open('synthetic.jpg',await fixture('jpeg'),'image/jpeg');
    let imported=await download();
    assert.equal(imported.width,64);assert.equal(imported.height,48);
    assert.ok(pixel(imported,30,30).every((n,i)=>Math.abs(n-[240,32,64][i])<12));
    await open('synthetic.bmp',bmpFixture(),'image/bmp');
    imported=await download();
    assert.deepEqual(imported.pixels,saved.pixels);
    for(const [name,data,mime] of [
      ['synthetic.tiff',tiffFixture(),'image/tiff'],
      ['bad.png',Buffer.from('not a picture'),'image/png'],
      ['empty.png',Buffer.alloc(0),'image/png'],
      ['too-large.png',await fixture('png',4096,1025),'image/png']]) {
      await open(name,data,mime);
      assert.deepEqual((await download()).pixels,imported.pixels,`${name}: drawing preserved`);
    }
    report.tests.push('PNG roundtrip, JPEG and BMP import, TIFF/malformed/empty/oversize preserve drawing');
    await page.mouse.click(241,box.y+43); await tick();
    await page.screenshot({path:path.join(output,'zoom-in.png')});
    let zoomed=await decode(path.join(output,'zoom-in.png'),{x:origin.x+68,y:origin.y+25});
    assert.deepEqual(zoomed,[240,32,64]);
    assert.deepEqual((await download()).pixels,imported.pixels);
    await shortcut('Control+0');
    await page.mouse.click(274,box.y+43); await tick();
    await page.screenshot({path:path.join(output,'zoom-out.png')});
    zoomed=await decode(path.join(output,'zoom-out.png'),{x:origin.x+60,y:origin.y+25});
    assert.notDeepEqual(zoomed,[240,32,64]);
    await shortcut('Control+0');
    await shortcut('1');await drag(origin.x+5,origin.y+44,origin.x+50,origin.y+44);
    assert.deepEqual(pixel(await download(),20,44),[224,32,48]);
    report.tests.push('zoom in/out/reset changes view only, freehand pixel output');
    await page.screenshot({path:path.join(output,'imported-and-drawn.png')});
    await page.setViewportSize({width:390,height:844});await tick();
    const narrow=await page.locator('canvas').first().boundingBox();
    assert.ok(narrow.width<=390);
    await newImage(96,72,true,true);
    await shortcut('1');
    const touch=await context.newCDPSession(page);
    const point={x:6,y:narrow.y+149};
    await touch.send('Input.dispatchTouchEvent',{type:'touchStart',touchPoints:[point]});
    for(let step=1;step<=8;step++) {
      await touch.send('Input.dispatchTouchEvent',{type:'touchMove',touchPoints:[{x:6+step*5,y:point.y}]});
    }
    await touch.send('Input.dispatchTouchEvent',{type:'touchEnd',touchPoints:[]});await tick();
    saved=await download();
    assert.equal(saved.width,96);assert.equal(saved.height,72);
    assert.deepEqual(pixel(saved,20,10),[224,32,48]);
    await page.screenshot({path:path.join(output,'narrow-touch.png')});
    report.tests.push('390x844 viewport, toolbar new/keyboard download, touch stroke pixels (emulated)');



  }
  assert.deepEqual(errors, [], 'JS/Wasm/HTTP errors');
  report.tests.push('startup without JS/Wasm/HTTP errors');
  await fs.writeFile(path.join(output,'report.json'),JSON.stringify(report,null,2));
} catch(error) {
  report.error=String(error);await fs.writeFile(path.join(output,'report.json'),JSON.stringify(report,null,2));
  await page.screenshot({path:path.join(output,'failure.png')}).catch(()=>{});
  throw error;
} finally { await browser.close(); if(server) server.close(); }
