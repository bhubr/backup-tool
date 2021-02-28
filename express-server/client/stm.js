let n = 0

class Rectangle {
  static stack = []

  constructor (w, h) {
    this.w = w
    this.h = h
    this.direction = this.h < this.w ? 'row' : 'column'
    this.children = []
  }

  layoutrow (row) {
    console.log('>>> layout', row)
    const rowArea = row.reduce((c, r) => c + r, 0)
    const otherSide = rowArea / this.width()
    let w
    let h
    if (this.direction === 'row') {
      h = '100%'
      w = `${otherSide * 100 / this.w}%`
    } else {
      w = '100%'
      h = `${otherSide * 100 / this.h}%`
    }
    console.log('<<< layout', otherSide, w, h)
  }

  width () {
    return Math.min(this.w, this.h)
  }
}

const rect = new Rectangle(6, 4)

function worst (R, w) {
  const s = R.reduce((c, r) => c + r, 0)
  const sq = s ** 2
  const wq = w ** 2
  const rMin = Math.min(...R)
  const rMax = Math.max(...R)
  console.log(Math.max(wq * rMax / sq, sq / (wq * rMin)))
  return Math.max(wq * rMax / sq, sq / (wq * rMin))
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
    rect.layoutrow(row)
    squarify(children, [], rect.width())
  }
}

squarify([6, 6, 4, 3, 2, 2, 1], [], rect.width())
