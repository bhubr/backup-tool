let n = 0

function worst (R, w) {
  const s = R.reduce((c, r) => c + r, 0)
  const sq = s ** 2
  const wq = w ** 2
  const rMin = Math.min(...R)
  const rMax = Math.max(...R)
  console.log(Math.max(wq * rMax / sq, sq / (wq * rMin)))
  return Math.max(wq * rMax / sq, sq / (wq * rMin))
}

function layoutrow (row) {
  console.log('should layout', row)
}

function width () {

}

function squarify (children, row, w) {
  n++
  console.log('sq', children, row, n)
  if (n > 15) return
  const c = children[0]
  const r1 = worst(row, w)
  const r2 = worst([...row, c], w)
  console.log('ratios', r1, r2)
  if ((r1 === 0) || (r1 > r2)) {
    squarify(children.slice(1), [...row, c], w)
  } else {
    layoutrow(row)
    squarify(children, [], width())
  }
}

squarify([6, 6, 4, 3, 2, 2, 1], [], 4)
