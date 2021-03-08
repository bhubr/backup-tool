const MAX = 100
let n = 0

export class Rectangle {
  static stack = []
  static lastInStack = () => Rectangle.stack[Rectangle.stack.length - 1]
  static pushToStack = (rect) => Rectangle.stack.push(rect)
  static clearStack = () => {
    Rectangle.stack = []
  }

  constructor (w, h, options = {}) {
    this.w = w
    this.h = h
    this.direction = options.direction || (this.h < this.w ? 'row' : 'column')
    this.children = []
    this.label = options.label || 'N/A'
  }

  layoutrow (row, getArea = c => c) {
    const rowArea = row.reduce((c, r) => c + getArea(r), 0)
    const otherSide = rowArea / this.width()
    let w
    let h
    let children
    let firstRect
    let secondRect
    if (this.direction === 'row') {
      w = otherSide
      children = row.map(it => new Rectangle(w, this.h * (getArea(it) / rowArea), { label: it.path }))
      firstRect = new Rectangle(otherSide, this.h, { direction: 'column' })
      secondRect = new Rectangle(this.w - otherSide, this.h)
    } else {
      h = otherSide
      children = row.map(it => new Rectangle(this.w * (getArea(it) / rowArea), h, { label: it.path }))
      firstRect = new Rectangle(this.w, otherSide, { direction: 'row' })
      secondRect = new Rectangle(this.w, this.h - otherSide)
    }
    console.log('layoutrow', firstRect, secondRect)
    Rectangle.pushToStack(secondRect)
    firstRect.children = children
    this.children.push(firstRect)
    this.children.push(secondRect)
  }

  width () {
    return Math.min(this.w, this.h)
  }
}

function worst (R, w, getArea) {
  console.log('>> worst R/w', R, w);
  if (R.length === 0) return +Infinity
  const s = R.reduce((c, r) => c + getArea(r), 0)
  const areas = R.map(it => getArea(it))
  const sq = s ** 2
  const wq = w ** 2
  const rMin = Math.min(...areas)
  const rMax = Math.max(...areas)
  // console.log('>> worst s/sq/areas', s, sq, areas);
  // console.log('>> worst w/wq/rmin/rmax', w, wq, rMin, rMax)
  console.log('>> worst ret');
  console.log(Math.max(wq * rMax / sq, sq / (wq * rMin)))
  return Math.max(wq * rMax / sq, sq / (wq * rMin))
}

function squarify (children, row, w, getArea = c => c) {
  n++
  if (n >= MAX) {
    console.log('ABORT', n)
    return
  }
  // console.log('>> squarify', n, children, row, w)
  if (!children.length) {
    // console.log('>> squarify DONE')
    Rectangle.clearStack()
    return
  }
  const rect = Rectangle.lastInStack()
  const c = children[0]
  const r1 = worst(row, w, getArea)
  const r2 = worst([...row, c], w, getArea)
  console.log(r1, r2)
  if (row.length === 0 || (r1 > r2)) {
    children.shift()
    squarify(children, [...row, c], w, getArea)
  } else {
    rect.layoutrow(row, getArea)
    squarify(children, [], rect.width(), getArea)
  }
}

function getStyle (w, h, direction, hue, ratio) {
  const sat = 70
  const lig = 50
  return {
    display: 'flex',
    flexDirection: direction,
    width: `${w * ratio}px`,
    height: `${h * ratio}px`,
    background: `hsl(${hue}, ${sat}%, ${lig}%)`,
    boxShadow: 'inset 0px 0px 4px 0px rgba(0, 0, 0, 0.5)'
  }
}

const getHue = (() => {
  let hue = 0
  return () => {
    const ret = hue
    hue = (hue + 2) % 360
    return ret
  }
})()

export function renderToDOMNode (node, children, options) {
  const pixelRatio = options.pixelRatio || 4
  const getArea = options.getArea || function(c) { return c }
  console.log('renderToDOMNode', getArea.toString())
  n = 0
  const width = 400
  const area = children.reduce((c, a) => c + getArea(a), 0)
  const height = area / width
  console.log('>> renderToDOMNode', area, width, height)
  const rect = new Rectangle(width, height)
  node.innerHTML = '';
  Rectangle.pushToStack(rect)
  console.log('>> renderToDOMNode rect/width', rect, rect.width())
  squarify(children, [], rect.width(), getArea)
  console.log('>> renderToDOMNode squarify done', rect)
  node.appendChild(renderAsDOMNode(rect, pixelRatio, getArea))
}

function renderAsDOMNode(rectangle, pixelRatio, getArea) {
  console.log('>> renderAsDOMNode start', rectangle)
  const hue = getHue()
  const div = document.createElement('DIV')
  const style = getStyle(rectangle.w, rectangle.h, rectangle.direction, hue, pixelRatio)
  console.log('>> renderAsDOMNode', style)
  Object.keys(style).forEach(k => div.style[k] = style[[k]])
  div.addEventListener('click', (e) => {
    e.stopPropagation()
    console.log(rectangle)
  })
  rectangle.children.forEach((child, i) => {
    const childNode = renderAsDOMNode(child, pixelRatio, getArea)
    childNode.title = child.label
    div.appendChild(childNode)
  })
  return div
}
