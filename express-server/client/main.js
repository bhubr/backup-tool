import io from 'socket.io-client'
import { renderToDOMNode } from './stm.js'
const socket = io('http://localhost:5000')
const scans = []
// let ts1
// let ts2

socket.on('scan:start', msg => {
  const { id: driveId, label, timestamp } = JSON.parse(msg)
  scans
    .filter(s => s.driveId === driveId && !s.done)
    .forEach(s => { s.done = true })
  console.log(scans)

  const scanId = `${driveId}:${timestamp}`
  scans.push({
    scanId,
    driveId,
    label,
    done: false,
    start: Date.now(),
    progress: {
      estimate: null,
      scan: null
    },
    filesStart: null,
    stmFolders: []
  })
  const li = document.createElement('LI')
  li.innerText = scanId
  document.querySelector('#current-id').innerText = scanId
  document.querySelector('#current-phase').innerText = ''
  document.querySelector('#scans').appendChild(li)
  document.querySelector('#pc').innerText = '% completed'
  document.querySelector('#progress-value').style.width = '0'
  document.querySelector('#time').innerText = '(time in seconds)'
  document.querySelector('#files').innerHTML = ''
})

socket.on('scan:stats', msg => {
  const { id: driveId, files, dirs } = JSON.parse(msg)
  const scan = scans
    .find(s => s.driveId === driveId && !s.done)
  if (!scan) console.error('no scan!')
  if (!scan.filesStart) scan.filesStart = Date.now()
  else {
    const elapsed = (Date.now() - scan.filesStart) / 1000
    document.querySelector('#current-dirs').innerText = dirs.toString() + ` (${(dirs / elapsed).toFixed(1)}/sec)`
    document.querySelector('#current-files').innerText = files.toString() + ` (${(files / elapsed).toFixed(1)}/sec)`
  }
})

const copyAndSortTreeMapItems = (arr, getArea = c => c) => {
  const copy = [...arr]
  return copy.sort((a, b) => getArea(b) - getArea(a))
}

socket.on('scan:dir-stats', msg => {
  const { id: driveId, path, count } = JSON.parse(msg)
  const scan = scans
    .find(s => s.driveId === driveId && !s.done)
  if (!scan) console.error('no scan!')
  scan.stmFolders.push({ path, count })
  // console.log('>>>>> RENDER TREEMAP', scan.stmFolders)

  if (scan.stmFolders.length % 10 === 0) renderToDOMNode(
  // if (scan.stmFolders.length === 20) renderToDOMNode(
    document.querySelector('#treemap'),
    copyAndSortTreeMapItems(scan.stmFolders, it => it.count),
    { getArea: it => it.count }
  );
})

socket.on('percent', msg => {
  const { id: driveId, percent, phase } = JSON.parse(msg)
  const scan = scans
    .find(s => s.driveId === driveId && !s.done)
  if (!scan) console.error('no scan!')

  if (!scan.progress[phase]) scan.progress[phase] = { start: Date.now() }
  const now = Date.now()
  document.querySelector('#current-phase').innerText = phase
  document.querySelector('#pc').innerText = `${percent}%`
  document.querySelector('#progress-value').style.width = `${percent}%`
  document.querySelector('#time').innerText = `(${((now - scan.progress[phase].start) / 1000).toFixed(1)} seconds)`
})

socket.on('scan:done', msg => {
  document.querySelector('#files').innerHTML = ''
  const scan = scans
    .find(s => s.driveId === driveId && !s.done)
  if (!scan) console.error('no scan!')

  renderToDOMNode(document.querySelector('#treemap'), copyAndSortTreeMapItems(scan.stmFolders, it => it.count), { getArea: it => it.count });
  console.log('DONE', scan.id)
  // const files = JSON.parse(msg)
  // files.forEach(f => {
  //   const li = document.createElement('LI')
  //   li.innerText = f
  //   document.querySelector('#files').appendChild(li)
  // })
})
