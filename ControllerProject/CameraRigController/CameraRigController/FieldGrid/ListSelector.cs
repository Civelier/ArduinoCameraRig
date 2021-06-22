using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CameraRigController.FieldGrid
{
    public class ListSelector<T> : ObservableCollection<T>, INotifyPropertyChanged
    {
        private readonly ObservableCollection<T> _innerCollection;

        public ListSelector()
        {
            _innerCollection = new ObservableCollection<T>();
            _innerCollection.CollectionChanged += _innerCollection_CollectionChanged;
        }


        public ListSelector(IEnumerable<T> enumeration)
        {
            _innerCollection = new ObservableCollection<T>(enumeration);
            _innerCollection.CollectionChanged += _innerCollection_CollectionChanged;
        }

        public ListSelector(List<T> list)
        {
            _innerCollection = new ObservableCollection<T>(list);
            _innerCollection.CollectionChanged += _innerCollection_CollectionChanged;
        }

        private void _innerCollection_CollectionChanged(object sender, NotifyCollectionChangedEventArgs e)
        {
            OnCollectionChanged(e);
        }

        protected override void ClearItems()
        {
            _innerCollection.Clear();
        }

        protected override void InsertItem(int index, T item)
        {
            _innerCollection.Insert(index, item);
        }

        protected override void MoveItem(int oldIndex, int newIndex)
        {
            _innerCollection.Move(oldIndex, newIndex);
        }

        protected override void OnCollectionChanged(NotifyCollectionChangedEventArgs e)
        {
            base.OnCollectionChanged(e);
        }

        protected override void RemoveItem(int index)
        {
            _innerCollection.RemoveAt(index);
        }

        protected override void SetItem(int index, T item)
        {
            _innerCollection[index] = item;
        }
    }
}
