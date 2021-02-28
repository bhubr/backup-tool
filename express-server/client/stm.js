export class Rectangle {
  static stack = []
  static lastInStack = () => Rectangle.stack[Rectangle.stack.length - 1]
  static pushToStack = (rect) => Rectangle.stack.push(rect)
  static clearStack = () => {
    Rectangle.stack = []
  }

  constructor (w, h, direction) {
    this.w = w
    this.h = h
    this.direction = direction || (this.h < this.w ? 'row' : 'column')
    this.children = []
    console.log('>> Rectangle ctor', w, h, direction, this.direction)
  }

  layoutrow (row) {
    const rowArea = row.reduce((c, r) => c + r, 0)
    const otherSide = rowArea / this.width()
    console.log('>>> layout', row, rowArea, otherSide)
    let w
    let h
    let children
    let firstRect
    let secondRect
    if (this.direction === 'row') {
      w = otherSide
      // h = this.w
      children = row.map(area => console.log('subrect', area, area / rowArea, w, this.h * (area / rowArea)) || new Rectangle(w, this.h * (area / rowArea)))
      firstRect = new Rectangle(otherSide, this.h, 'column')
      secondRect = new Rectangle(this.w - otherSide, this.h)
    } else {
      // throw new Error('dunno')
      h = otherSide
      // w = this.h
      children = row.map(area => new Rectangle(this.w * (area / rowArea), h))
      firstRect = new Rectangle(this.w, otherSide, 'row')
      secondRect = new Rectangle(this.w, this.h - otherSide)
      // firstRect = new Rectangle(otherSide, this.h, 'row')
    }
    Rectangle.pushToStack(secondRect)
    console.log('1st/2nd', firstRect, secondRect)
    firstRect.children = children
    this.children.push(firstRect)
    this.children.push(secondRect)

    // console.log('<<< layout', otherSide, w, h)
  }

  width () {
    return Math.min(this.w, this.h)
  }
}

function worst (R, w) {
  const s = R.reduce((c, r) => c + r, 0)
  const sq = s ** 2
  const wq = w ** 2
  const rMin = Math.min(...R)
  const rMax = Math.max(...R)
  return Math.max(wq * rMax / sq, sq / (wq * rMin))
}

function squarify (children, row, w) {
  console.log('>> squarify', children, row, w)
  if (!children.length && !row.length) {
    console.log('DONE')
    Rectangle.clearStack()
    return
  }
  const rect = Rectangle.lastInStack()
  const c = children[0]
  const r1 = worst(row, w)
  const r2 = worst([...row, c], w)
  if ((r1 === 0) || (r1 > r2)) {
    children.shift()
    squarify(children, [...row, c], w)
  } else {
    rect.layoutrow(row)
    squarify(children, [], rect.width())
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

export function renderToDOMNode (node, children, pixelRatio = 3) {
  const width = 500
  const area = children.reduce((c, a) => c + a, 0)
  const height = area / width
  console.log('>> renderToDOMNode', area, width, height)
  const rect = new Rectangle(width, height)
  node.innerHTML = '';
  node.appendChild(renderAsDOMNode(children, rect, pixelRatio))
}

function renderAsDOMNode(children, rectangle, pixelRatio, level = 0) {
  const hue = getHue()
  Rectangle.pushToStack(rectangle)
  squarify(children, [], rectangle.width())
  const div = document.createElement('DIV')
  const style = getStyle(rectangle.w, rectangle.h, rectangle.direction, hue, pixelRatio)
  Object.keys(style).forEach(k => div.style[k] = style[[k]])
  div.addEventListener('click', (e) => {
    e.stopPropagation()
    console.log(rectangle)
  })
  rectangle.children.forEach((child, i) => {
    const childNode = renderAsDOMNode(children, child, pixelRatio, level + 1)
    div.appendChild(childNode)
  })
  return div
}
