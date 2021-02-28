import io from 'socket.io-client'
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
    }
  })
  const li = document.createElement('LI')
  li.innerText = scanId
  document.querySelector('#current-id').innerText = scanId
  document.querySelector('#current-phase').innerText = ''
  document.querySelector('#scans').appendChild(li)
  document.querySelector('#pc').innerText = '% completed'
  document.querySelector('#progress-value').style.width = '0'
  document.querySelector('#time').innerText = '(time in seconds)'
})

socket.on('scan:stats', msg => {
  console.log(msg)
  const { files, dirs } = JSON.parse(msg)
  document.querySelector('#current-dirs').innerText = dirs.toString()
  document.querySelector('#current-files').innerText = files.toString()
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

socket.on('files', msg => {
  document.querySelector('#files').innerHTML = ''
  const files = JSON.parse(msg)
  files.forEach(f => {
    const li = document.createElement('LI')
    li.innerText = f
    document.querySelector('#files').appendChild(li)
  })
})
