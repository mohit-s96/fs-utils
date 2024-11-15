const fs = require("fs");

const mine = fs.readFileSync("mine.txt").toString();
const find = fs.readFileSync("find.txt").toString();

const changeToArray = (text) => {
  return text.split("\nfoo");
};

(() => {
  const mineArray = changeToArray(mine);
  const findArray = changeToArray(find);

  findArray.forEach((line) => {
    if (!mineArray.includes(line)) {
      console.log(line);
    }
  });
})();
